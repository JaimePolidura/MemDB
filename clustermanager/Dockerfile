FROM  golang:1.20.2

WORKDIR /clustermanager

COPY . /clustermanager

RUN cd ./src && go install && go build

ENTRYPOINT ["/clustermanager/src/src"]