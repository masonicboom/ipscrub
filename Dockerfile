FROM ubuntu:latest

WORKDIR .

RUN apt-get update && apt-get install -y curl gcc make git libpcre3-dev zlib1g-dev libbsd-dev

ADD ipscrub /ipscrub/
ADD Makefile /

EXPOSE 8081

RUN make nginx/objs/nginx

CMD make start && tail -f dest/logs/access.log