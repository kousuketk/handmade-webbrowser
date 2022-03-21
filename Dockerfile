FROM ubuntu:20.04

# wget, make, gcc, libpng, libfreetype6をインストール
RUN apt-get update && apt-get install -y wget
RUN apt install -y make
RUN apt install -y build-essential
RUN apt install -y libpng-dev
RUN apt-get install libfreetype6

RUN apt install sudo  
RUN apt-get install -y vim
COPY src/ app/

RUN wget http://download.savannah.gnu.org/releases/freetype/\freetype-2.10.4.tar.gz
RUN tar xvf freetype-2.10.4.tar.gz
WORKDIR  /freetype-2.10.4
RUN ./configure
RUN make
RUN make install
WORKDIR /app