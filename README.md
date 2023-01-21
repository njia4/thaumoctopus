# Thaumoctopus
A lightweight DRM based preview. 

## Description

This is a very simple c++ library for displaying and overlaying frames obtained from a camera or a video stream *without* an X server. I developed it so I could have a camera preview and some overlay images with transparency on the Raspberry Pi Zero board. The limited resources on a Pi Zero make it difficult to run any GUI windows, and overlay with transpanrency using ```GStreamer``` is also pretty costy for some reason I don't understand. So I built this little piece of code to do a light weight display and overlay using ```libdrm```. 

## Installation

I have only tested this on a Raspi Zero W board with XXX, so it is possible extra steps are needed. 

* Download the package

  ```shell
  git clone https://github.com/njia4/thaumoctopus
  ```

* Install ```libdrm```

  ```shell
  sudo apt-get install libdrm
  ```

* Make 

  ```shell
  mkdir build && cd build
  cmake ..
  make
  ```

* Install the package (optional)

  ```shell
  make install
  ```

## Usage

