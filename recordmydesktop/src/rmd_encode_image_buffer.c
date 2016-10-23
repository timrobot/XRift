/******************************************************************************
*                            recordMyDesktop                                  *
*******************************************************************************
*                                                                             *
*            Copyright (C) 2006,2007,2008 John Varouhakis                     *
*                                                                             *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA  *
*                                                                             *
*                                                                             *
*                                                                             *
*   For further information contact me at johnvarouhakis@gmail.com            *
******************************************************************************/

#include "config.h"
#include "rmd_encode_image_buffer.h"
#include "shmdata.h"

#include "rmd_types.h"

#include <errno.h>

static int flag;
shmdata * data;
void *rmdEncodeImageBuffer(ProgData *pdata){
	if (flag == 0)
	{
	  key_t key;
	  int shmflg;
	  int shmid;
	  int size;

	  key = 9000;
	  size = sizeof(shmdata);
	  shmflg = 0666 | IPC_CREAT;

	  // get an shm
	  if ((shmid = shmget(key, size, shmflg)) == -1) {
		print_error("[SOURCE] shmget failed, trying again");
		struct shmid_ds sds;
		shmctl(shmid, IPC_RMID, &sds);
		if ((shmid = shmget(key, size, shmflg)) == -1) {
		  print_error("[SOURCE] shmget failed again, failing");
		  //return 1;
		} else {
		  print_debug("[SOURCE] Got an shm!");
		}
	  } else {
		print_debug("[SOURCE] Got an shm!");
	  }

	  // attach the shm to this process
	  data = (shmdata *)shmat(shmid, data, shmflg);
	  if (data == (shmdata *)-1) {
		print_error("[SOURCE] failed to attach");
		//return 1;
	  } else {
		print_debug("[SOURCE] found character stream");
	  }
	  flag = 1;
	}
    pdata->th_encoding_clean=0;
    while(pdata->running){
        pdata->th_enc_thread_waiting=1;
        pthread_mutex_lock(&pdata->img_buff_ready_mutex);
        pthread_cond_wait(&pdata->image_buffer_ready,
                          &pdata->img_buff_ready_mutex);
        pthread_mutex_unlock(&pdata->img_buff_ready_mutex);
        pdata->th_enc_thread_waiting=0;
        pdata->encoder_busy = TRUE;
        if (pdata->paused) {
            pthread_mutex_lock(&pdata->pause_mutex);
            pthread_cond_wait(&pdata->pause_cond, &pdata->pause_mutex);
            pthread_mutex_unlock(&pdata->pause_mutex);
        }
        pthread_mutex_lock(&pdata->yuv_mutex);
        memcpy(data, &pdata->enc_data->yuv, 1980*1080*2);  
        pthread_mutex_unlock(&pdata->yuv_mutex);
        //printf("Got encode image buf\n");
        //pthread_mutex_unlock(&pdata->yuv_mutex);

        /*if(theora_encode_YUVin(&pdata->enc_data->m_th_st,
                               &pdata->enc_data->yuv)){
            fprintf(stderr,"Encoder not ready!\n");
            pthread_mutex_unlock(&pdata->yuv_mutex);
        }
        else{
            pthread_mutex_unlock(&pdata->yuv_mutex);
            if(theora_encode_packetout(&pdata->enc_data->m_th_st,0,
                                       &pdata->enc_data->m_ogg_pckt1)==1){
                pthread_mutex_lock(&pdata->libogg_mutex);
                ogg_stream_packetin(&pdata->enc_data->m_ogg_ts,
                                    &pdata->enc_data->m_ogg_pckt1);
                pthread_mutex_unlock(&pdata->libogg_mutex);
                pdata->avd+=pdata->frametime;
            }
        }*/
        pdata->encoder_busy = FALSE;
    }
  

    //last packet
    pdata->th_encoding_clean=1;
    pthread_mutex_lock(&pdata->theora_lib_mutex);
    pthread_cond_signal(&pdata->theora_lib_clean);
    pthread_mutex_unlock(&pdata->theora_lib_mutex);
    pthread_exit(&errno);
}

//this function is meant to be called normally
//not through a thread of it's own
void rmdSyncEncodeImageBuffer(ProgData *pdata){
    if(theora_encode_YUVin(&pdata->enc_data->m_th_st,
                            &pdata->enc_data->yuv)){
        fprintf(stderr,"Encoder not ready!\n");
    }
    else{
        if(theora_encode_packetout(&pdata->enc_data->m_th_st,!pdata->running,
                                    &pdata->enc_data->m_ogg_pckt1)==1){
            pthread_mutex_lock(&pdata->libogg_mutex);
            ogg_stream_packetin(&pdata->enc_data->m_ogg_ts,
                                &pdata->enc_data->m_ogg_pckt1);
            if(!pdata->running)pdata->enc_data->m_ogg_ts.e_o_s=1;
            pthread_mutex_unlock(&pdata->libogg_mutex);
            pdata->avd+=pdata->frametime;
        }
    }
}
