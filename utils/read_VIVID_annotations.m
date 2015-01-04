function gt = read_VIVID_annotations(seq_idx)

root_dir = '../data/Groundtruth';

% read ground truth
filename = sprintf('%s/%s_truth.csv', root_dir, seq_idx);
fid = fopen(filename, 'r');

% skip the first line
header = fgets(fid);
% read the following
C = textscan(fid, '%s %d %d %f %f %f %f','delimiter', ',');

% build the gt structure
gt.x = C{4};
gt.y = C{5};
gt.w = C{6};
gt.h = C{7};
gt.fr = C{2};
gt.vid = C{3};