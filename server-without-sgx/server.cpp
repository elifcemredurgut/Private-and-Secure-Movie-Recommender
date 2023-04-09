#include <iostream>
#include <string>
#include "zmq_server.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <list>
#include <set>

std::map<int, std::map<int, double>> ratingsPerNeighbor;
std::map<int, std::list<int>> mapOfIndices;
std::map<int, std::list<double>> movieRatings;
std::map<int, double> movieRatingAvg;
std::map<int, std::list<int>> userProfile;
std::map<int, int> userRequestCount;
std::set<int> neighborsCommunicatedSoFar;

ZmqServer zmqServer = ZmqServer();

bool isCompleted(int userID){
    for (auto n = mapOfIndices.at(userID).begin(); n != mapOfIndices.at(userID).end(); n++) {
        auto pos = neighborsCommunicatedSoFar.find(*n);
        if(pos == neighborsCommunicatedSoFar.end()){
            return false;
        }
    }
    return true;
}

void recommend(int key){
    // find the average movie ratings
    for (int n : mapOfIndices.at(key)){
        
        for (auto pair : ratingsPerNeighbor.at(n)){
            int movieID = pair.first;
            double rating = pair.second;

            // remove the movies that the requester has watched before
            std::list<int>::iterator it;
            it = std::find(userProfile.at(key).begin(), userProfile.at(key).end(), movieID);

            if(it == userProfile.at(key).end()){ //if movie has not been watched
                if(movieRatings.count(movieID) == 0) { //if movie is not in the map
                    std::list<double> l;
                    l.push_back(rating);
                    movieRatings[movieID] = l;
                } else {
                    movieRatings[movieID].push_back(rating);
                }
            }
        }
    }
    for (auto p : movieRatings) {
        // Key is an integer
        int key = p.first;

        // Value is a list of integers
        std::list<double> ratings = p.second;

        double sum = 0;
        int count = 0;
        for (auto r = ratings.begin(); r != ratings.end(); r++) {
            // Dereferencing value pointed by iterator
            sum += *r;
            count++;
        }
        std::string sum_s = std::to_string(sum);
        std::string count_s = std::to_string(count);

        movieRatingAvg[key] = sum/count;
    }

    //get the top 5 movies
    std::vector<std::pair<int, int>> top_five(5);
    std::partial_sort_copy(movieRatingAvg.begin(),
        movieRatingAvg.end(),
        top_five.begin(),
        top_five.end(),
        [](std::pair<const int, int> const& l,
        std::pair<const int, int> const& r)
        {
            return l.second > r.second;
        }
    );

    std::string moviesToBeSent = "";
    for (int i=0; i<5; i++)
    {
        int mov = top_five[i].first;
        moviesToBeSent += std::to_string(mov)+",";
    }
    zmqServer.sendmore("client"+std::to_string(key));
    zmqServer.send("movies"+moviesToBeSent);
    //send("client"+std::to_string(key), "movies"+moviesToBeSent);

    //clear the data
    movieRatings.clear();
    movieRatingAvg.clear();
}

bool isNeighborStillNeeded(int id){
	for(auto u : userProfile){
		std::list<int>::iterator it;
		it = std::find(mapOfIndices.at(u.first).begin(), mapOfIndices.at(u.first).end(), id);
		if(it != mapOfIndices.at(u.first).end()){
			return true;
		}
	}
	return false;
}

void requestProfileAgain(int id){

    for(auto user_movie : userProfile){
        int user = user_movie.first;
        std::list<int>::iterator it;
        it = std::find(mapOfIndices.at(user).begin(), mapOfIndices.at(user).end(), id);
        if(it != mapOfIndices.at(user).end()){
            auto pos = neighborsCommunicatedSoFar.find(id);
            if(pos == neighborsCommunicatedSoFar.end()){
                zmqServer.sendmore("client"+std::to_string(id));
		zmqServer.send("send");
            }
        }
    }	    
}

int main(){
    int numberOfClientsNeedToBeDone = 10;
    std::ifstream ifs("knn_top10.txt");
    if(ifs){
        std::string content( (std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        std::stringstream ss(content);
        std::string line;

        while (std::getline(ss, line, '\n')) {
            std::string index;
            std::list<int> listOfIndices;

            std::size_t pos = line.find(","); //index of the first comma
            std::string indicesStr = line.substr(pos+1);  //rest of the line after the first element(userId)
            std::stringstream indicesStream(indicesStr);

            int userId = std::stoi(line.substr(0, pos));  //part of the line until the first comma

            while(std::getline(indicesStream, index, ',')){
                listOfIndices.push_back(std::stoi(index));
            }
            mapOfIndices[userId] = listOfIndices;
        }
    }
    else{
        std::cout << "Cannot open knn_top10.txt file!\n";
    }

    std::cout << "Listening on 5555" << std::endl;
    int numberOfClientsDone = 0;

    while(numberOfClientsDone < numberOfClientsNeedToBeDone){
	//std::cout << numberOfClientsDone << std::endl;
        std::string socketID = zmqServer.receive().substr(6); //message format is client<ID>
        int userID = std::stoi(socketID);
        std::string msg = zmqServer.receive();
        
        // -------------------------------------------------------------------------------------------
        //request for movie recommendations
        if(msg.substr(0,5) == "recom"){
           
            // message format: recom<movie1>,<movie2>,...
            std::string movies = msg.substr(5);
            std::string m;

            std::list<int> listOfUserProfile;
            while(movies != ""){
                auto index_comma = movies.find_first_of(",");
                m = movies.substr(0, index_comma);
                listOfUserProfile.push_back(std::stoi(m));

                movies = movies.substr(index_comma+1, movies.length());
            }

	    //update the request count
            userProfile[userID] = listOfUserProfile;
	    if(userRequestCount.find(userID) == userRequestCount.end()){
		userRequestCount[userID] = 1;
	    }
	    else{
		userRequestCount[userID] += 1;
	    }

            // iterate over the neighbors list
            for (auto n = mapOfIndices.at(userID).begin(); n != mapOfIndices.at(userID).end(); n++) {
                auto pos = neighborsCommunicatedSoFar.find(*n);
                if(pos == neighborsCommunicatedSoFar.end()){
                    zmqServer.sendmore("client"+std::to_string(*n));
                    zmqServer.send("send");
                }
            }

            std::map<int, std::list<int>>::iterator itr;
            for (auto itr = userProfile.cbegin(); itr != userProfile.cend();) {
                int key = itr->first; // userID that sent recommendation request

                if(isCompleted(key)){
		    numberOfClientsDone++;
		    recommend(key);
		    userRequestCount[key] -= 1;
		    if(userRequestCount[key] <= 0){
		    	userProfile.erase(itr++);
		    }
		    for(auto n : mapOfIndices.at(key)){
			if(!isNeighborStillNeeded(n)){
			    neighborsCommunicatedSoFar.erase(n);
			    ratingsPerNeighbor.erase(n);
			}
		    }
                }
                else {
                    ++itr;
                }
            }
        }
        // -------------------------------------------------------------------------------------------
        //incoming user profiles
        else if(msg == "Hello"){
            zmqServer.sendmore("client"+std::to_string(userID));
            zmqServer.send("OK");

            //check-up
            //
            //some clients may send recom requests before their neighbors connect
            //coverage for the "send" messages that don't reach to their recipients
            
            requestProfileAgain(userID);
        }
        // -------------------------------------------------------------------------------------------
        else if (msg.substr(0,5) == "movie"){

            neighborsCommunicatedSoFar.insert(userID);

            // msg format: movie<movie1>:<rating1>,<movie2>:<rating2>,...
            std::string content = msg.substr(5);
            std::string pair;

            // split the message by comma ","
            while(content != ""){
                int index_comma = content.find_first_of(",");
                pair = content.substr(0, index_comma);

                content = content.substr(index_comma+1, content.length());

                std::size_t colonPos = pair.find(":");
                int movieID = std::stoi(pair.substr(0, colonPos));
                double rating = std::stod(pair.substr(colonPos+1));

                ratingsPerNeighbor[userID][movieID] = rating;
            }

            std::map<int, std::list<int>>::iterator itr;
            for (auto itr = userProfile.cbegin(); itr != userProfile.cend();) {
                int key = itr->first; // userID that sent recommendation request

                if(isCompleted(key)){
                    numberOfClientsDone++;
                    recommend(key);
                    userRequestCount[key] -= 1;
                    if(userRequestCount[key] <= 0){
                        userProfile.erase(itr++);
                    }
		    for(auto n : mapOfIndices.at(key)){
                        if(!isNeighborStillNeeded(n)){
                            neighborsCommunicatedSoFar.erase(n);
                            ratingsPerNeighbor.erase(n);
                        }
                    }
                }
                else {
                    ++itr;
                }
            }
        }
        // -------------------------------------------------------------------------------------------
        else {
            std::cout << "Invalid request from the client!" << std::endl;
        }
    }

    return 0;
}
