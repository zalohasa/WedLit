#include "Player.h"

#include <thread>
#include <iostream>
#include <fmt123.h>

#define BITS 8

void Player::play_internal(Player* player)
{
	size_t done;
	int err;
	ao_sample_format format;
    int channels, encoding;
    long rate;

	mpg123_handle* mh = mpg123_new(NULL, &err);
	size_t buffer_size = mpg123_outblock(mh);
    unsigned char* buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    if (mpg123_open(mh, "/home/zalo/tmp.mp3") != MPG123_OK)
    {
    	std::cout << "Error reading file" << std::endl;
    }

    mpg123_format_none(mh);
    mpg123_format(mh, 48000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* decode and play */
    try{
    	int error;
	    while ((error = mpg123_read(mh, buffer, buffer_size, &done)) == MPG123_OK)
	    {
	        ao_play(player->dev_, (char*)buffer, done);
	    }
	    if (error != MPG123_OK)
	    {
	    	std::cout << "Error code: " << error << std::endl;
	    }
	} 
	catch(const std::exception& ex)
	{
		std::cout << "Exception!!!" << std::endl;
		std::cout << ex.what() << std::endl;
	}
	std::cout << "End of the thread" << std::endl;
	mpg123_close(mh);
	mpg123_delete(mh);
}

Player::Player(const std::string& filename) : 
	IPlayer(),
	filename_(filename), 
	stop_(false)
{
    
    int err;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    mpg123_init();
    int nDrivers;
    ao_info** infos = ao_driver_info_list(&nDrivers);
    for (int i = 0; i < nDrivers; ++i)
    {
    	std::cout << "Name: " << infos[i]->name << std::endl;
    	std::cout << "Short name: " << infos[i]->short_name << " Comm: " << infos[i]->comment << std::endl;
    }

    int driver = ao_default_driver_id();
    //int driver = ao_driver_id("pulse");
    ao_info* info = ao_driver_info(driver);
    std::cout << "Selected driver: " << std::endl;
    std::cout << "Name: " << info->name << " Comm: " << info->comment << std::endl;
    mpg123_handle* mh = mpg123_new(NULL, &err);
	
    /* open the file and get the decoding format */
    mpg123_open(mh, "/home/zalo/tmp.mp3");
    //
    mpg123_format_none(mh);
    mpg123_format(mh, 48000, MPG123_STEREO, MPG123_ENC_SIGNED_16);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;

    std::cout << "Rate: " << format.rate << std::endl;

    dev_ = ao_open_live(driver, &format, NULL);
    if (!dev_)
    {
    	std::cout << "Cant open device: " << std::endl;
    	switch(errno)
    	{
    		case AO_ENODRIVER:
    			std::cout << "No driver" << std::endl;
    			break;
    		case AO_ENOTLIVE:
    			std::cout << "No live output" << std::endl;
    			break;
    		case AO_EBADOPTION:
    			std::cout << "Wrong option value" << std::endl;
    			break;
    		case AO_EOPENDEVICE:
    			std::cout << "Cannot open the device" << std::endl;
    			break;
    		case AO_EFAIL:
    			std::cout << "Unknown error" << std::endl;
    			break;
    	}
    }

    mpg123_close(mh);
    mpg123_delete(mh);
}

Player::~Player()
{
	/* clean up */
    ao_close(dev_);
    mpg123_exit();
    ao_shutdown();
}

bool Player::play()
{	
	if (dev_)
	{
		bck_ = std::thread(&Player::play_internal, this);
		bck_.detach();
		return true;
	}
	exit(1);
	return false;
}

bool Player::stop()
{
	return true;
}

