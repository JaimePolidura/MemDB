docker build --progress=plain -f Dockerfile-base -t memdb-core-base:1.0 .
docker tag memdb-core-base:1.0 jaimetruman/memdb-core-base:1.0
docker push jaimetruman/memdb-core-base:1.0
