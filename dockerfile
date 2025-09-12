# Stage 1: Build with Emscripten
FROM emscripten/emsdk:3.1.56 AS builder

WORKDIR /app

# Copy project
COPY . .

# Clean previous builds
RUN rm -rf build

# Configure project with emcmake
RUN emcmake cmake -B build -DCMAKE_CXX_STANDARD=17

# Build project
RUN em++ src/*.cpp -o rizz.js \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ENVIRONMENT=web \
    -s ALLOW_MEMORY_GROWTH \
    -s EXPORTED_FUNCTIONS="['_runCodeC']" \
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
    -std=c++17



# Create a single .wasm module from the built executable (assuming target is 'rizz')
RUN emcc src/*.cpp \
    -o rizz.js \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=0 \
    -s EXPORTED_FUNCTIONS="['_runCodeC']" \
    -s EXPORTED_RUNTIME_METHODS="['cwrap','ccall']"


# Stage 2: Export final wasm (and JS loader)
FROM scratch AS export
COPY --from=builder /app/rizz.wasm /
COPY --from=builder /app/rizz.js /
