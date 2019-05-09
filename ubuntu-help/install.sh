###############################################################################
# Basic
###############################################################################
sudo apt-get update
sudo apt-get upgrade
###############################################################################
# gvim
###############################################################################
sudo apt-get install vim-gtk3
which gvim
###############################################################################
# terminator
###############################################################################
sudo apt-get install terminator
which terminator
###############################################################################
# Octave
###############################################################################
sudo apt-get install octave
sudo apt-get install octave-communications
sudo apt-get install octave-image
which octave
###############################################################################
# Partition Tool
###############################################################################
sudo apt-get install gparted
which gparted
###############################################################################
# Google Drive TODO
###############################################################################
sudo apt-get install grive2
sudo apt-get install grive
###############################################################################
# Play Music (Not so great. Removed it)
###############################################################################
sudo apt-get install snapd
sudo snap install google-play-music-desktop-player
sudo snap remove google-play-music-desktop-player
###############################################################################
# gcc
###############################################################################
sudo apt-get install gcc
gcc -v
# gcc/g++ alternatives
sudo update-alternatives --config gcc
sudo update-alternatives --config g++
###############################################################################
# slack
###############################################################################
sudo apt-get install slack
sudo snap install slack --classic
###############################################################################
# R
###############################################################################
sudo apt install apt-transport-https software-properties-common
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
sudo add-apt-repository 'deb https://cloud.r-project.org/bin/linux/ubuntu bionic-cran35/'
sudo apt update
sudo apt install r-base
sudo apt-get install build-essential
R --version
which Rscript 
###############################################################################
# WireShark
###############################################################################
sudo apt-get install wireshark
which wireshark
###############################################################################
# AWS CLI
###############################################################################
sudo apt-get install awscli
which aws
###############################################################################
# Python3
###############################################################################
sudo apt-get install python3
# Python CSVKIT
sudo apt-get install python3-csvkit
sudo apt-get install csvkit
# ipython3
sudo apt-get install ipython3
which ipython3
# python3 pip
sudo apt-get install python3-pip
# Python3 libraries
pip3 install numpy
pip3 install matplotlib
pip3 install tkinter
sudo apt-get install python3-tk
###############################################################################
# gimp
###############################################################################
sudo apt-get install gimp
which gimp
###############################################################################
# TkCVS/TkDiff
###############################################################################
sudo apt-get install tkcvs
sudo apt-get install tkdiff
which tkcvs
###############################################################################
# meld
###############################################################################
sudo apt-get install meld
###############################################################################
# REAL VNC VIEWER
###############################################################################
# remove tightvnc viewer first
sudo apt-get -y remove xtightvncviewer
# download from realvnc website
tar -xvf VNC-6.0.2-Linux-x64-DEB.tar.gz
sudo dpkg -i VNC-Viewer-6.0.2-Linux-x64.deb
# If not able to login to vncservers check if router is restricting connections
###############################################################################
# Google Chrome
###############################################################################
sudo apt-get install -y chromium-browser
###############################################################################
# GIT
###############################################################################
sudo apt install git
git clone https://github.com/psrivatsa/linux.git
git config --global user.name psrivatsa
git config --global user.email "prashanth.srivatsa@gmail.com"

###############################################################################
# HW Information
###############################################################################
sudo apt install hwinfo
hwinfo
sudo apt install lsscsi
lsscsi
