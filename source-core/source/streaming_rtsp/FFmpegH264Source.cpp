//READ LICENSE BEFORE ANY USAGE
/* Copyright (C) 2018  Damien DUBUC ddubuc@aneo.fr (ANEO S.A.S)
 *  Team Contact : hipe@aneo.fr
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *  
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 *  In addition, we kindly ask you to acknowledge ANEO and its authors in any program 
 *  or publication in which you use HIPE. You are not required to do so; it is up to your 
 *  common sense to decide whether you want to comply with this request or not.
 *  
 *  Non-free versions of HIPE are available under terms different from those of the General 
 *  Public License. e.g. they do not require you to accompany any object code using HIPE 
 *  with the corresponding source code. Following the new licensing any change request from 
 *  contributors to ANEO must accept terms of re-license by a general announcement. 
 *  For these alternative terms you must request a license from ANEO S.A.S Company 
 *  Licensing Office. Users and or developers interested in such a license should 
 *  contact us (hipe@aneo.fr) for more information.
 */

//
//  FFmpegH264Source.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

#include <FFmpegH264Source.h>
#include <GroupsockHelper.hh>
#include <core/misc.h>

namespace MESAI
{
    FFmpegH264Source * FFmpegH264Source::createNew(UsageEnvironment& env, FFmpegH264Encoder * E_Source) {
		return new FFmpegH264Source(env, E_Source);
	}

	FFmpegH264Source::FFmpegH264Source(UsageEnvironment& env, FFmpegH264Encoder * E_Source) : FramedSource(env), Encoding_Source(E_Source)
	{
		m_eventTriggerId = envir().taskScheduler().createEventTrigger(FFmpegH264Source::deliverFrameStub);
		std::function<void()> callback1 = std::bind(&FFmpegH264Source::onFrame,this);
		Encoding_Source -> setCallbackFunctionFrameIsReady(callback1);
	}

	FFmpegH264Source::~FFmpegH264Source()
	{

	}

	void FFmpegH264Source::doStopGettingFrames()
	{
        FramedSource::doStopGettingFrames();
	}

	void FFmpegH264Source::onFrame()
	{
		envir().taskScheduler().triggerEvent(m_eventTriggerId, this);
	}

	void FFmpegH264Source::doGetNextFrame()
	{
		deliverFrame();
	}

	void FFmpegH264Source::deliverFrame()
	{
		if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

		static uint8_t* newFrameDataStart;
		static unsigned newFrameSize = 0;

		/* get the data frame from the Encoding thread.. */
		if (Encoding_Source->GetFrame(&newFrameDataStart, &newFrameSize)){
			if (newFrameDataStart!=NULL) {
				/* This should never happen, but check anyway.. */
				if (newFrameSize > fMaxSize) {
					fFrameSize = fMaxSize;
					fNumTruncatedBytes = newFrameSize - fMaxSize;
				} else {
					fFrameSize = newFrameSize;
				}

				hipe_gettimeofday(&fPresentationTime, NULL);
				memcpy(fTo, newFrameDataStart, fFrameSize);
				
				//delete newFrameDataStart;
				//newFrameSize = 0;
				
				Encoding_Source->ReleaseFrame();
			}
			else {
				fFrameSize=0;
				fTo=NULL;
				handleClosure(this);
			}
		}else
		{
			fFrameSize = 0;
		}
		
		if(fFrameSize>0)
			FramedSource::afterGetting(this);

	}
}