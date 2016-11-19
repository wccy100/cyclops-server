﻿/********************************************************************
  Created by MythKAst
  ©2013 MythKAst Some rights reserved.


  You can build it with vc2010,gcc.
  Anybody who gets this source code is able to modify or rebuild it anyway,
  but please keep this section when you want to spread a new version.
  It's strongly not recommended to change the original copyright. Adding new version
  information, however, is Allowed. Thanks.
  For the latest version, please be sure to check my website:
  Http://code.google.com/mythkast


  你可以用vc2010,gcc编译这些代码
  任何得到此代码的人都可以修改或者重新编译这段代码，但是请保留这段文字。
  请不要修改原始版权，但是可以添加新的版本信息。
  最新版本请留意：Http://code.google.com/mythkast
  B
 MythKAst(asdic182@sina.com), in 2013 June.
*********************************************************************/
#include "mythAvlist.hh"
#include <stdio.h>
#include <memory.h>
mythAvlist::mythAvlist(void)
	:mythVirtualList()
{
	listcount = 0;
	this->startread = false;
	this->totalptr = 0;
	mBufferSize = AVBUFFERSIZE;
	InitalList();
}
int mythAvlist::InitalList(){
	//inital list
	totalbuffer = new unsigned char[mBufferSize * 1024 * 1024];
	ListPacket = new PacketQueue[AVFRAMECOUNT];
	for (int i = 0; i < AVFRAMECOUNT; i++){

		//ListPacket[i].YY = NULL;
		//ListPacket[i].UU = NULL;
		//ListPacket[i].VV = NULL;
		ListPacket[i].h264Packet = NULL;
		ListPacket[i].h264PacketLength = 0;

		for (int j = 0; j < 3; j++){
			ListPacket[i].yuvPacket[j] = NULL;
			ListPacket[i].yuvPacketLength[j] = 0;
		}
	}
	listwrite = 0;
	listread = 0;
	return 0;
}
mythAvlist::mythAvlist(int BufferSize)
{
	listcount = 0;
	this->startread = false;
	mBufferSize = BufferSize;
	//totalbuffer = (unsigned char*)SDL_malloc(BufferSize * 1024 * 1024);
	totalptr = 0;
	InitalList();
}
mythAvlist *mythAvlist::CreateNew(int BufferSize){
	if(BufferSize == 0)
		return new mythAvlist();
	else
		return new mythAvlist(BufferSize);
}
mythAvlist::~mythAvlist(void)
{
	free();
}
PacketQueue *mythAvlist::get(int freePacket){
	_mutex.lock();
	PacketQueue *tmp;
	if (this->listwrite - this->listread == 1 ||
		this->listwrite - this->listread == 0 ||
		(this->listwrite == 0 && this->listread == AVFRAMECOUNT)){
		tmp = NULL;
	}
	else{
		tmp = &this->ListPacket[this->listread];
		if (tmp->h264Packet == NULL && tmp->yuvPacket[0] == NULL){
			tmp = NULL;
		}
		else{
			if (freePacket == 0){
				if (listwrite - listread > 10){
					LOGE("skip frames");
					LOGE(" read = %d,write = %d,minus = %d\n", listread, listwrite, listwrite - listread);
					listread += 9;
				}
				else
					listread++;
			}
		}
	}
	listread %= AVFRAMECOUNT;
	_mutex.unlock();
	return tmp;
}
unsigned char* mythAvlist::putcore(unsigned char* data,unsigned int datasize){
	if(totalptr + datasize > (unsigned int)(mBufferSize * 1024 * 1024))
		totalptr = 0;
	memcpy(totalbuffer + totalptr, data, datasize);
	totalptr += datasize;
	//printf("totalptr = %d\n",totalptr);
	return (totalbuffer + totalptr - datasize);
}

int mythAvlist::put(unsigned char** dataline,unsigned int *datasize,int width,int height){
	_mutex.lock();
	if(listwrite >= AVFRAMECOUNT)listwrite = 0;
	PacketQueue *tmp = &ListPacket[listwrite];
	tmp->h264Packet = NULL;
	//tmp->width = width;
	//tmp->height = height;

	unsigned char* YY = (unsigned char*)putcore(dataline[0],datasize[0] * height);
	unsigned int Ydatasize = datasize[0];
	
	unsigned char* UU = (unsigned char*)this->putcore(dataline[1], datasize[1] * height / 2);
	unsigned int Udatasize = datasize[1];
	
	unsigned char* VV = (unsigned char*)this->putcore(dataline[2], datasize[2] * height / 2);
	unsigned int Vdatasize = datasize[2];
	tmp->yuvPacket[0] = YY; tmp->yuvPacket[1] = UU; tmp->yuvPacket[2] = VV;
	tmp->yuvPacketLength[0] = Ydatasize; tmp->yuvPacketLength[1] = Udatasize; tmp->yuvPacketLength[2] = Vdatasize;
	listwrite++;
	//LOGE("YUVlistcount=%d\n",listwrite);
	_mutex.unlock();
	return 0;
}

int mythAvlist::release(PacketQueue *pack)
{
	return 0;
}

int mythAvlist::put(unsigned char* data,unsigned int length){	
	_mutex.lock();
	if(listwrite >= AVFRAMECOUNT)listwrite = 0;
	PacketQueue *tmp = &ListPacket[listwrite];

	//tmp->YY = NULL;
	//tmp->UU = NULL;
	//tmp->VV = NULL;

	tmp->h264PacketLength = length;
	tmp->h264Packet = putcore(data, length);
	tmp->isIframe = IsIframe(tmp);
	listwrite++;
	//LOGE("H264listcount=%d\n",listwrite);
	_mutex.unlock();
	return 0;
}
int mythAvlist::free(){
	if (ListPacket)
		delete [] ListPacket;
	ListPacket = NULL;
	if (totalbuffer)
		delete [] totalbuffer;
	totalbuffer = NULL;
	return 0;
}
