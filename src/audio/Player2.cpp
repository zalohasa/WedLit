#include "Player2.h"
#include "WedLit.h"

#include <thread>
#include <iostream>
#include <fmt123.h>

INIT_LOG(PLAYER2);

#define BITS 8

void Player2::play_internal(Player2* player)
{
	size_t done;

	size_t buffer_size = mpg123_outblock(player->mh_);
    unsigned char* buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    
    {
        //Notify others I'm about to play....
        std::unique_lock<std::mutex>(player->lock_);
        player->playing_ = true;
        player->cond_.notify_one();
    }
  
    /* decode and play */
    while ((!(player->stop_)) && (mpg123_read(player->mh_, buffer, buffer_size, &done) == MPG123_OK))
    {
        ao_play(player->dev_, (char*)buffer, done);
    }

    free((void*)buffer);

    /* clean up */
    ao_close(player->dev_);
    mpg123_close(player->mh_);
    mpg123_delete(player->mh_);
    DEBUG("End of the audio player thread");
}

Player2::Player2(const std::string& filename) : 
	IPlayer(),
	filename_(filename), 
	stop_(false),
    playing_(false)
{
    int err;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    
    int nDrivers;

    int driver = ao_default_driver_id();
    ao_info* info = ao_driver_info(driver);
    DEBUG("Audio driver selected: {}", info->name);
    mh_ = mpg123_new(NULL, &err);
	
    /* open the file and get the decoding format */
    if (mpg123_open(mh_, filename.c_str()) != MPG123_OK)
    {
        ERROR("Error opening file {}", filename);
    }

    //Setting the format 48000hz sample rate
    //TODO make this adjustable via config.
    mpg123_format_none(mh_);
    mpg123_format(mh_, 48000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
    mpg123_getformat(mh_, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;

    dev_ = ao_open_live(driver, &format, NULL);
    if (!dev_)
    {
    	std::string errorStr;
    	switch(errno)
    	{
    		case AO_ENODRIVER:
    			errorStr = "No driver";
    			break;
    		case AO_ENOTLIVE:
    			errorStr = "No live output";
    			break;
    		case AO_EBADOPTION:
    			errorStr = "Wrong option value";
    			break;
    		case AO_EOPENDEVICE:
    			errorStr = "Cannot open the device";
    			break;
    		case AO_EFAIL:
    			errorStr = "Unknown error";
    			break;
    	}
        ERROR("Cant open audio device: {}", errorStr);
    }
}

void Player2::waitForFinish()
{
    if (bck_.joinable())
    {
        DEBUG("Waiting for audio thread to finish");
        bck_.join();
    }
}

Player2::~Player2()
{
    //Wait for audio thread to finish
    stop_ = true;
    if (bck_.joinable())
    {
        DEBUG("Waiting for audio thread to finish");
        bck_.join();
    }
	
}

bool Player2::play()
{	
	if (dev_ && mh_)
	{
		bck_ = std::thread(&Player2::play_internal, this);
        
        //Wait for the thread to be about to start playing
        std::unique_lock<std::mutex> lk(lock_);
        cond_.wait(lk, [this]{return playing_;});
        lk.unlock();

		return true;
	}
	return false;
}

bool Player2::stop()
{
    stop_ = true;
	return true;
}

void Player2::init()
{
    /* initializations */
    ao_initialize();
    mpg123_init();
}

void Player2::shutdown()
{
    mpg123_exit();
    ao_shutdown();
}
