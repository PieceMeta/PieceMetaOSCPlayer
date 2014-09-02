//
//  APIClient.cpp
//  PieceMetaOSCPlayer
//
//  Created by Anton on 01/09/14.
//
//

#include "APIClient.h"

data_package_t APIClient::loadUrl(std::string url)
{
    complete = false;
    data_package_t result;
    unsigned int maxFrames = 0;
    float frameRate = .0;
    bool loadPackageSuccess;
    ofxJSONElement dataPackage;
    ofxJSONElement dataChannels;
    
    try
    {
        loadPackageSuccess = dataPackage.open(url);
    }
    catch(int e)
    {
        ofLogNotice("APIClient::find") << "exception while loading json: " << e << '\n';
        return result;
    }
    
    if (loadPackageSuccess)
    {
        if (!dataPackage.isObject() || dataPackage.isMember("code")) {
            ofLogNotice("APIClient::find")  << "data package load error for package url: " << url << endl;
            return result;
        } else {
            result.title = dataPackage["title"].asString();
            result.packageId = dataPackage["id"].asString();
        }
    }
    else
    {
        ofLogNotice("APIClient::find")  << "data package parse error for package url: " << url << endl;
        return result;
    }
    
    if (dataChannels.open(url + "/channels"))
    {
        for(unsigned int c = 0; c < dataChannels.size(); ++c)
        {
            
            data_channel_t dataChannel;
            ofxJSONElement dataStreams;
            
            dataChannel.title = dataChannels[c]["title"].asString();
            dataChannel.channelId = dataChannels[c]["id"].asString();
            
            if (dataStreams.open(url + "/channels/" + dataChannels[c]["id"].asString() + "/streams")) {
                
                std::map<std::string, data_stream_t> streams;
                
                for(unsigned int s = 0; s < dataStreams.size(); ++s)
                {
                    if (streams.find(dataStreams[s]["group"].asString()) == streams.end())
                    {
                        data_stream_t dataStream;
                        dataStream.streamId = dataStreams[s]["id"].asString();
                        dataStream.title = dataStreams[s]["title"].asString();
                        dataStream.group = dataStreams[s]["group"].asString();
                        
                        unsigned int frameCount = dataStreams[s]["data_frames"].size();
                        if (frameCount > maxFrames) {
                            maxFrames = frameCount;
                        }
                        if (dataStreams[s]["frame_rate"].asFloat() > frameRate) {
                            frameRate = dataStreams[s]["frame_rate"].asFloat();
                        }
                        for(unsigned int v = 0; v < frameCount; ++v)
                        {
                            std::map<std::string, float> frameValues;
                            frameValues[dataStreams[s]["title"].asString()] = dataStreams[s]["data_frames"][v].asFloat();
                            dataStream.dataFrames.push_back(frameValues);
                        }
                        
                        streams[dataStream.group] = dataStream;
                    }
                    else
                    {
                        for(unsigned int f = 0; f < streams[dataStreams[s]["group"].asString()].dataFrames.size(); ++f)
                        {
                            streams[dataStreams[s]["group"].asString()].dataFrames[f][dataStreams[s]["title"].asString()] = dataStreams[s]["data_frames"][f].asFloat();
                        }
                    }
                }
                
                for (std::map<std::string, data_stream_t>::iterator i = streams.begin(); i != streams.end(); i++)
                {
                    dataChannel.dataStreams.push_back(i->second);
                }
                
                result.dataChannels.push_back(dataChannel);
                
            }
            else
            {
                ofLogNotice("APIClient::find")  << "data streams parse error for package url: " << url << endl;
            }
            
        }
    }
    else
    {
        ofLogNotice("APIClient::find")  << "data channels parse error for package url: " << url << endl;
        return result;
    }
    
    result.frameRate = frameRate;
    result.frameCount = maxFrames;
    
    ofLogNotice("APIClient::find") << "successfully fetched data package with id: " << result.packageId << endl;
    
    complete = true;
    return result;
}