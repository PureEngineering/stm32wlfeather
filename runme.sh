ZEPHYR_VERSION=v3.3.0
zephyr_sdk_version=0.16.0



zephyr_release_setup_description="install Zephyr ($ZEPHYR_VERSION) and SDK"
zephyr_release_setup()
{
    # install zephyrrtos (release) in home directory
    CURR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
    cd ~

    # ZEPHYR_DIR=zephyr_$ZEPHYR_VERSION
    ZEPHYR_DIR=zephyr_$ZEPHYR_VERSION
    if [ -d $ZEPHYR_DIR ]
    then
        echo "ZephyrRTOS ($ZEPHYR_VERSION) already exists in home directory"
    else

        mkdir $ZEPHYR_DIR && cd $ZEPHYR_DIR

        sudo apt-get install --no-install-recommends git cmake ninja-build gperf \
            ccache dfu-util device-tree-compiler wget \
            python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
            make gcc gcc-multilib g++-multilib libsdl2-dev -y
            
        /usr/bin/python3 -m pip install --upgrade pip
        pip3 install --user -U west
        echo 'export PATH=~/.local/bin:"$PATH"' >> ~/.bashrc
        source ~/.bashrc
        pip3 install --user pynrfjprog

        # install ZephyrRTOS
        west init -m https://github.com/zephyrproject-rtos/zephyr --mr $ZEPHYR_VERSION
        west update
        west zephyr-export
        pip3 install --user -r zephyr/scripts/requirements.txt

        # fixes the builing issue for cryptography module
        pip3 install --upgrade pip
        pip3 install setuptools-rust
        
        pip3 install --user -r bootloader/mcuboot/scripts/requirements.txt    
    fi

    # download and install zephyr-sdk toolchain
    cd ~
    zephyr_sdk_filename=zephyr-sdk-$zephyr_sdk_version\_linux-x86_64.tar.xz
    if test -f "zephyr-sdk-$zephyr_sdk_version"; then
        echo "Sdk already installed"
    else
        wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v$zephyr_sdk_version/$zephyr_sdk_filename
        tar xvf $zephyr_sdk_filename
        cd zephyr-sdk-$zephyr_sdk_version
        chmod +x $zephyr_sdk_filename
        ./setup.sh
    fi

    # Go back to the directory
    cd $CURR_DIR
}

source common/automenu.sh