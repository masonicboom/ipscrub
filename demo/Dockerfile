FROM ubuntu:latest

RUN apt-get update && apt-get install -y curl

CMD while true; do curl --silent http://nginx:8081/ > /dev/null; sleep 0.5; done