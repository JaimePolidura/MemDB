docker build --progress=plain -t memdb-core:1.0 .
docker tag memdb-core:1.0 jaimetruman/memdb-core:1.0
docker push jaimetruman/memdb-core:1.0
