function idl = read_VIVID_annotations_idl(seq_idx)

root_dir = '../data/Groundtruth';
img_dir = '../data/VIVID';

% read ground truth
filename = sprintf('%s/%s_truth.csv', root_dir, seq_idx);
fid = fopen(filename, 'r');

% skip the first line
header = fgets(fid);
% read the following
C = textscan(fid, '%s %d %d %f %f %f %f','delimiter', ',');

% image directory
image_dir = fullfile(img_dir, seq_idx);
filenames = dir(fullfile(image_dir, '*.jpg'));
num = length(filenames);

% build the IDL structure
idl = struct('bb', cell(num, 1), 'img', cell(num, 1), 'score', cell(num, 1), 'id', cell(num,1));
for i = 1:num
    index = C{2} == i - 1;
    idl(i).bb = [C{4}(index) C{5}(index) C{4}(index)+C{6}(index) C{5}(index)+C{7}(index)];
    idl(i).id = C{3}(index);
    
    % default scores
    idl(i).score = -1 * ones(size(idl(i).bb,1),1);
    
    % image name
    idl(i).img = filenames(i).name;
end