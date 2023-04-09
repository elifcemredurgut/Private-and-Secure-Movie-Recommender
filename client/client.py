import zmq
import sys
from csv import reader


context = zmq.Context()
# Socket to talk to server
socket = context.socket(zmq.DEALER)

id = sys.argv[1]
iid = int(id)
identity = "client" + id

socket.setsockopt_string(zmq.IDENTITY, identity)
socket.connect("tcp://localhost:5555")
socket.send_string("Hello")

count = 0
numberOfRequestsNeedToBeSent = 1

while(True):
    message = socket.recv()

    encoding = 'utf-8'
    m = message.decode(encoding)

    if(m == "send"):
        profile = "movie"
        with open('ratings.csv', 'r') as read_obj:
            csv_reader = reader(read_obj)
            header = next(csv_reader)
            if header != None:
                # Iterate over each row after the header in the csv
                for row in csv_reader:
                    # row variable is a list that represents a row in csv
                    if row[0] == id:
                        profile += row[1] + ":" + row[2] + ","
        socket.send_string(profile)

    elif(m == "OK"):
        while count < numberOfRequestsNeedToBeSent and (iid < 11):
            msg = "recom"
            with open('ratings.csv', 'r') as read_obj:
                csv_reader = reader(read_obj)
                header = next(csv_reader)
                if header != None:
                    # Iterate over each row after the header in the csv
                    for row in csv_reader:
                        # row variable is a list that represents a row in csv
                        if row[0] == id:
                            msg += row[1] + ","
            socket.send_string(msg)
            count += 1

    elif(m[:6] == "movies"):
        rest = m[6:-1] #last char is ","
        movies = rest.split(",")
        result = "Recommendations for client" + id + ":\n"
        numOfMovies = 0
        with open('movies.csv', 'r') as read_obj:
            csv_reader = reader(read_obj)
            header = next(csv_reader)
            if header != None:
                # Iterate over each row after the header in the csv
                for row in csv_reader:
                    # row variable is a list that represents a row in csv
                    if row[0] in movies:
                        result += row[0] + " --> " + row[1] + "\n"
                        numOfMovies += 1
                        if numOfMovies == 5:
                            break
        print(result, "\n")
