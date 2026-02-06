# ORCA Framework 

## Installation

### Linux

To install the required dependencies on **Linux**, run the following commands:

```sh
sudo apt update && sudo apt install -y \
    libpng-dev \
    libfreetype6-dev \
    libjpeg-dev \
    liblua5.4-dev \
    libxml2-dev \
    liblz4-dev \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libwayland-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev
```

For **Fedora**, use:

```sh
sudo dnf install -y \
    libpng-devel \
    freetype-devel \
    libjpeg-turbo-devel \
    lua-devel \
    libxml2-devel \
    lz4-devel \
    zlib-devel \
    libcurl-devel
```

For **Arch Linux**, use:

```sh
sudo pacman -S libpng freetype2 libjpeg-turbo lua libxml2 lz4 zlib curl
```

### macOS

To install dependencies on **macOS**, use **Homebrew**:

```sh
brew install libpng freetype jpeg lua libxml2 lz4 zlib curl
```

## Building the Project

To build the project, run:

```sh
make
```

To run the project after building, use:

```sh
make run
```

For a clean build and immediate execution, use:

```sh
make andrun
```

This will clean the build, compile the project (which takes 3-5 seconds), and immediately run the application with the `../icui` data folder.

### Required Data Folder

To run the project, ensure that the `../icui` folder is present, as it contains necessary data files. Reach out to **@icherna** to obtain the required data packages.

## Usage

(Explain how to use the project here)

## License

This project is licensed under the MIT License - see the [LICENCE](LICENCE) file for details.

