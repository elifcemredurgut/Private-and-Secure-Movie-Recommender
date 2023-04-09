#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

std::map<int, std::map<int, double>> ratingsPerNeighbor;
std::map<int, std::list<int>> mapOfIndices;
std::map<int, std::list<double>> movieRatings;
std::map<int, double> movieRatingAvg;
std::map<int, std::list<int>> userProfile;
std::map<int, int> userRequestCount;
std::set<int> neighborsCommunicatedSoFar;
/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

void send(std::string id, std::string msg)
{
    char *idchar = &id[0];
    char *msgchar = &msg[0];
    ocall_send(idchar, msgchar);
}

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
    send("client"+std::to_string(key), "movies"+moviesToBeSent);

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
		send("client"+std::to_string(id), "send");
	    }
	}
    }	    
}

void knnGraph(std::string input)
{
    std::string line = "";
    while (input != "")
    {
        auto index_newline = input.find_first_of("\n");
        line = input.substr(0, index_newline);

        //get the user ID
        auto index_first_comma = line.find_first_of(",");
        int userId = std::stoi(line.substr(0, index_first_comma));

        //rest of the line after the ID
        std::string restLine = line.substr(index_first_comma+1, line.length());

        input = input.substr(index_newline+1, input.length());

        //get the movie indices that belong to userId
        std::string movieIndex = "";
        std::list<int> listOfIndices;
        while(restLine.compare(movieIndex) != 0)
        {
                auto index_comma = restLine.find_first_of(",");
                movieIndex = restLine.substr(0, index_comma);
                listOfIndices.push_back(std::stoi(movieIndex));

                restLine = restLine.substr(index_comma+1, restLine.length());
        }
        mapOfIndices[userId] = listOfIndices;
    }
}

void ecall_init(const char *indices)
{
    printf("Inside of loadGraph!\n");
    std::string input(indices);
    knnGraph(input);
    int numberOfClientsDone = 0;
    printf("Knn graph has been saved!\n");

    int numberOfClientsNeedToBeDone = 10;

    while(numberOfClientsDone < numberOfClientsNeedToBeDone){
	
        char *idchar = (char*)malloc(10*sizeof(char));
        char *msgchar = (char*)malloc(20000*sizeof(char));
        ocall_receive(idchar, 10,  msgchar, 20000);
        std::string stringId(idchar);
        int userID = std::stoi(stringId.substr(6));
        std::string msg(msgchar);
        // -------------------------------------------------------------------------------------------
        // request for movie recommendations
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
                    send("client"+std::to_string(*n), "send");             
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
        // incoming user profiles
        else if(msg.substr(0,5) == "Hello"){
            send("client"+std::to_string(userID), "OK");
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

                // ratingsPerNeighbor[userID].insert(std::make_pair(movieID, rating));
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
		printf("Invalid request from the client!\n");
	}
    }
}

