# We need the osrm-backend lib to build trRouting so we rely on a fork of the official image
FROM greenscientist/osrm-backend:busterlib
WORKDIR /source
# Install dependencies in an intermediate image
RUN apt-get update && \
    apt-get -y --no-install-recommends install build-essential autoconf automake autoconf-archive pkg-config capnproto libcapnp-dev \
    libboost-all-dev libtool libncurses-dev

# Copy and build source    
COPY . /source    
RUN autoreconf -i && \
    ./configure && \
    make -j5 && \
    make install

# This CMD will run trRouting with default options
CMD trRouting

