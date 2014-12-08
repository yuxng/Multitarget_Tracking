cd utils
python generate_image_paths.py
python generate_confidence_paths.py
cd ..
./tracker data/imlist.txt data/conflist.txt
