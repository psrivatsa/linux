#####################################################################
# On Ubuntu 16.04 install gvim-GTK instead of gvim-gnome
# User should have root or sudo permissions
#####################################################################

# Remove gvim-gnome is already installed
sudo apt-get -y remove vim-gnome

# Install gvim-GTK
sudo apt-get install vim-gtk

# END
