#include <driver/i2s.h>


// === AUDIO MODES FOR THIS LIBRARY ===
#define AUDIO_MODE_PLAYER            0   // Only audio output (playback)
#define AUDIO_MODE_SAMPLER           1   // Only audio input (recording)
#define AUDIO_MODE_SAMPLER_PLAYER    2   // Both input and output (full-duplex)

// === WHICH ESP32 I2S HARDWARE UNITS TO USE ===
#define I2S_PORT   I2S_NUM_0         // First I2S controller
#define I2S_PORT_2 I2S_NUM_1         // Second I2S controller


int16_t eBuffer[PACKET_LENGTH_SAMPLES * 2];  //  we need that as buffer to expand a mono signal to stereo output (L and R outputs the same mono signal)

// === GLOBAL CONFIGURATION PARAMETERS ===
uint8_t audiochannelcount = 2;        // 1 = mono, 2 = stereo
uint8_t expandchannels = 0;           // If set, force stereo even if audiochannelcount == 1
uint8_t volumecontrol = 0;            // If set (=1), instead of 16 bits we are using 32 bit width
uint16_t samplerate = 32000;          // Sample rate in Hz

uint8_t i2sformat = 1;                // 1 = MSB, 2 = LSB (I2S communication format)
uint8_t samplebuffer_count = 4;       // Number of DMA buffers
uint8_t samplebuffer_length = 32;     // Buffer size per DMA buffer in audio frames

uint8_t AudioMode = 0;                // Set to one of AUDIO_MODE_* before starting
uint8_t externalclock = 0;            // external clocking of WS/LRCK and SCK/BCK

// For I2S pin definitions, you must define the following macros elsewhere:
// I2S_MCK_IN, I2S_SCK_IN, I2S_WS_IN, I2S_SD_IN, I2S_MCK_OUT, I2S_SCK_OUT, I2S_WS_OUT, I2S_SD_OUT


// https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/peripherals/i2s.html

// === MAIN I2S DRIVER START FUNCTION ===
void I2S_Start()
{
  // === Configure channel format based on mono/stereo and expansion ===
  // See: https://docs.espressif.com/projects/esp-idf/en/v3.3/api-reference/peripherals/i2s.html#_CPPv425I2S_CHANNEL_FMT_ONLY_LEFT
  i2s_channel_fmt_t audiochannelformat = I2S_CHANNEL_FMT_ONLY_LEFT; // default: mono (left)
  if (audiochannelcount == 2 || expandchannels == 1)
  {
    audiochannelformat = I2S_CHANNEL_FMT_RIGHT_LEFT; // stereo
  }

  // === Configure I2S communication format (MSB or LSB first) ===
  i2s_comm_format_t communicationformat = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
  if (i2sformat == 2)
  {
    communicationformat = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB);
	//communicationformat = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S);
  }

  // configuration for controlling the ADC
  i2s_config_t i2s_config_in;
  if (externalclock == 0)  // so called master mode, ESP32 makes the clock signal itself
  {	  	
	  // === I2S configuration for input (recording) ===
	  i2s_config_in = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),      // Master mode, RX only
		.sample_rate = samplerate,
		.bits_per_sample = i2s_bits_per_sample_t(16),           // 16-bit audio
		.channel_format = audiochannelformat,
		.communication_format = communicationformat,
		.intr_alloc_flags = 0,                                  // Default interrupt priority
		.dma_buf_count = samplebuffer_count,                    // Number of DMA buffers
		.dma_buf_len = samplebuffer_length,                     // Frames per DMA buffer
		.use_apll = true,                                       // Use APLL for better clock accuracy
		//.tx_desc_auto_clear = true, // not used for RX
		.fixed_mclk = samplerate * 256                          // Set MCLK to fs*256 (typical for I2S)
		//.mclk_multiple = I2S_MCLK_MULTIPLE_256                // Alternative: use enum, not available on all ESP-IDF versions
	  };
  }
  else   // // so called slave mode, you have to get at least the WS=LRCK and the SCK=BCK signal from an external clock
  {
		// === I2S configuration for input (recording) ===
	  i2s_config_in = {
		.mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX),      // Slave mode, RX only
		.sample_rate = samplerate,
		.bits_per_sample = i2s_bits_per_sample_t(16),           // 16-bit audio
		.channel_format = audiochannelformat,
		.communication_format = communicationformat,
		.intr_alloc_flags = 0,                                  // Default interrupt priority
		.dma_buf_count = samplebuffer_count,                    // Number of DMA buffers
		.dma_buf_len = samplebuffer_length,                     // Frames per DMA buffer
		.use_apll = false,                                       // Use APLL for better clock accuracy
		//.tx_desc_auto_clear = true, // not used for RX
		//.fixed_mclk = samplerate * 256                          // Set MCLK to fs*256 (typical for I2S)
		.mclk_multiple = I2S_MCLK_MULTIPLE_256                // Alternative: use enum, not available on all ESP-IDF versions
	  };

  }
 	
  //  configuration for controlling the DAC
  i2s_bits_per_sample_t bitspersample = (i2s_bits_per_sample_t)16;  // normally 16 bit

  if (volumecontrol == 1)
  {
	bitspersample = (i2s_bits_per_sample_t)32;	     // when controlling the volume with a factor which reduces the volume, we will output the signal with 32 bit, otherwise resolution gets lost
  }
  
  i2s_config_t i2s_config_out;
  if (externalclock == 0)  // so called master mode, ESP32 makes the clock signal itself
  {	  
	  // === I2S configuration for output (playback) ===
	  i2s_config_out = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),      // Master mode, TX only
		.sample_rate = samplerate,
		.bits_per_sample = bitspersample,
		.channel_format = audiochannelformat,
		.communication_format = communicationformat,
		.intr_alloc_flags = 0,
		.dma_buf_count = samplebuffer_count,
		.dma_buf_len = samplebuffer_length,
		.use_apll = true,                                       // use the in-built audio PLL of the ESP32
		.tx_desc_auto_clear = true,                             // Zeroes buffer on underrun (silence)
		.fixed_mclk = samplerate * 256
		//.mclk_multiple = I2S_MCLK_MULTIPLE_256 
	  };
  }
  else   // // so called slave mode, you have to get at least the WS=LRCK and the SCK=BCK signal from an external clock
  {
	  // === I2S configuration for output (playback) ===
	  i2s_config_out = {
		.mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_TX),      // Slave mode, TX only
		.sample_rate = samplerate,
		.bits_per_sample = bitspersample,
		.channel_format = audiochannelformat,
		.communication_format = communicationformat,
		.intr_alloc_flags = 0,
		.dma_buf_count = samplebuffer_count,
		.dma_buf_len = samplebuffer_length,
		.use_apll = false,
		.tx_desc_auto_clear = true,                             // Zeroes buffer on underrun (silence)
		//.fixed_mclk = samplerate * 256
		.mclk_multiple = I2S_MCLK_MULTIPLE_256 
	  };
  }


   // === Driver installation per selected audio mode ===
  if (AudioMode == AUDIO_MODE_PLAYER)
  {
    // Output only: Install driver on I2S_PORT with TX config
    i2s_driver_install(I2S_PORT, &i2s_config_out, 0, NULL);
  }

  if (AudioMode == AUDIO_MODE_SAMPLER)
  {
    // Input only: Install driver on I2S_PORT with RX config
    i2s_driver_install(I2S_PORT, &i2s_config_in, 0, NULL);
  }

  if (AudioMode == AUDIO_MODE_SAMPLER_PLAYER)
  {
    // Both: Install TX on I2S_PORT and RX on I2S_PORT_2
    i2s_driver_install(I2S_PORT,   &i2s_config_out, 0, NULL);
    i2s_driver_install(I2S_PORT_2, &i2s_config_in,  0, NULL);
  }

  // === Pin configuration: Define how I2S signals are mapped to ESP32 GPIOs ===
  const i2s_pin_config_t pin_config_in = {
    .mck_io_num = I2S_MCK_IN,   // Master clock input pin
    .bck_io_num = I2S_SCK_IN,   // Bit clock input pin
    .ws_io_num = I2S_WS_IN,     // Word select (LRCK) input pin
    .data_out_num = -1,         // No data out (input only)
    .data_in_num = I2S_SD_IN    // Data in pin
  };

  const i2s_pin_config_t pin_config_out = {
    .mck_io_num = I2S_MCK_OUT,  // Master clock output pin
    .bck_io_num = I2S_SCK_OUT,  // Bit clock output pin
    .ws_io_num = I2S_WS_OUT,    // Word select (LRCK) output pin
    .data_out_num = I2S_SD_OUT, // Data out pin
    .data_in_num = -1           // No data in (output only)
  };

	
  
  // === Pin setup and start per selected audio mode ===
  if (AudioMode == AUDIO_MODE_PLAYER)
  {
    i2s_set_pin(I2S_PORT, &pin_config_out);
    i2s_start(I2S_PORT);        // Start output I2S
  }

  if (AudioMode == AUDIO_MODE_SAMPLER)
  {
    i2s_set_pin(I2S_PORT, &pin_config_in);
    i2s_start(I2S_PORT);        // Start input I2S
  }

  if (AudioMode == AUDIO_MODE_SAMPLER_PLAYER)
  {
    i2s_set_pin(I2S_PORT, &pin_config_out);
    i2s_start(I2S_PORT);        // Start output on port 0

    i2s_set_pin(I2S_PORT_2, &pin_config_in);
    i2s_start(I2S_PORT_2);      // Start input on port 1
  }

 
}

// === STOP AND UNINSTALL THE I2S DRIVER ===
void I2S_Stop()
{
  i2s_stop(I2S_PORT);             // Stop port 0
  i2s_driver_uninstall(I2S_PORT); // Uninstall port 0 driver

  if (AudioMode == AUDIO_MODE_SAMPLER_PLAYER)
  {
    i2s_stop(I2S_PORT_2);            // Stop port 1
    i2s_driver_uninstall(I2S_PORT_2);// Uninstall port 1 driver
  }
}


// Restart the I2S interface. First stop it, then start it completely new
void I2S_Restart()
{
  I2S_Stop();
  delay(5);
  I2S_Start();
  delay(5);
}


// === QUICK RESET OF MAIN I2S PERIPHERAL (TX/RX FIFO RESET) ===
void I2S_Reset()
{
  i2s_stop(I2S_PORT);
  i2s_start(I2S_PORT);
  
}


// ----------------- WRITING / PLAYING ---------------------------------------


size_t bytesWritten = 0;


// Write NumberOfBytesToWrite Bytes from wBuffer to I2S output (blocking until done)
void I2S_WriteBytesFromBuffer(uint8_t* wBuffer, uint16_t NumberOfBytesToWrite)
{
  
  // Write to I2S DMA buffer (blocking until done)
	i2s_write(I2S_PORT, wBuffer, NumberOfBytesToWrite, &bytesWritten, portMAX_DELAY);
  
}


// Write NumberOfSamplesToWrite Samples from wBuffer to I2S output (blocking until done)
void I2S_WriteSamplesFromBuffer(int16_t* wBuffer, uint16_t NumberOfSamplesToWrite)
{
  if (expandchannels == 1)  // expand a mono signal to stereo output (L and R outputs the same mono signal)
  {
    for (uint8_t i = 0; i < NumberOfSamplesToWrite; i++)
    {
      eBuffer[i * 2] = wBuffer[i];
      eBuffer[i * 2 + 1] = wBuffer[i];
    }
    // Write to I2S DMA buffer (blocking until done)
	i2s_write(I2S_PORT, &eBuffer, NumberOfSamplesToWrite * 4, &bytesWritten, portMAX_DELAY);
  }
  else
  {
    // Write to I2S DMA buffer (blocking until done)
	i2s_write(I2S_PORT, wBuffer, NumberOfSamplesToWrite * 2, &bytesWritten, portMAX_DELAY);
  }
}

// Write NumberOfBytesToWrite from wBuffer to I2S output (blocking until done), this function is used to output 32-bit samples instead of 16-bit samples
// we can use 32 bit samples expanded from 16 bit samples, when we set a volume = multiplication factor without loosing dynamic of the audio signal
void I2S_WriteSamplesFromBuffer32(int32_t* wBuffer, uint16_t NumberOfSamplesToWrite)
{
  
  // Write to I2S DMA buffer (blocking until done)
  i2s_write(I2S_PORT, wBuffer, NumberOfSamplesToWrite * 4, &bytesWritten, portMAX_DELAY);
  
}


// --------------  READING / SAMPLING ----------------------------------

size_t bytesIn = 0;

// Read NumberOfBytesToRead Bytes from I2S DMA buffer
void I2S_ReadBytesInBuffer(uint8_t* rBuffer, uint16_t NumberOfBytesToRead)
{
  
  // Read Bytes from I2S DMA buffer, blocking until the data are available
  esp_err_t result = i2s_read(I2S_PORT, rBuffer, NumberOfBytesToRead, &bytesIn, portMAX_DELAY);
  
}


// Read NumberOfBytesToRead Bytes from I2S DMA buffer, this function is used in Sampler+Player mode, because we use then the second I2S interface to sample data
void I2S_2_ReadBytesInBuffer(uint8_t* rBuffer, uint16_t NumberOfBytesToRead)
{
  // Read Bytes from I2S DMA buffer, blocking until the data are available
  esp_err_t result = i2s_read(I2S_PORT_2, rBuffer, NumberOfBytesToRead, &bytesIn, portMAX_DELAY); 
}



