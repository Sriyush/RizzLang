# Use official emscripten SDK image
FROM emscripten/emsdk:3.1.56 as builder

WORKDIR /app

# Copy project
COPY . .
RUN rm -rf build
# Build to WebAssembly (C++17)
RUN emcmake cmake -DCMAKE_CXX_STANDARD=17 -B build
RUN emmake make -C build -j$(nproc)

# Example: if your main target is 'rizz'
RUN emcc src/main.cpp -o rizz.wasm \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ENVIRONMENT="web,node" \
    -std=c++17

# Final stage (optional, just outputs wasm)
FROM scratch AS export
COPY --from=builder /app/rizz.wasm /
