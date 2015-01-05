function dres = read_VIVID_detections(seq_idx)

detection_dir = '../data/Detections';
filename = fullfile(detection_dir, seq_idx, 'correlation_detections.csv');

% read detections
fid = fopen(filename, 'r');
C = textscan(fid, '%d %s %f %f %f');
fclose(fid);

width = 40;
height = 40;

dres.x = C{3}-width/2;
dres.y = C{4}-height/2;
dres.w = width*ones(size(C{3}));
dres.h = height*ones(size(C{3}));
dres.r = C{5};
dres.fr = C{1};