cmake_minimum_required(VERSION 3.13)
project(midi_ctl)

file(GLOB SRC "src/*.cpp")

include(${SDRPP_MODULE_CMAKE})