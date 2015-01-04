function idl = read_tracking_idl(filename, seq_idx)

fid = fopen(filename, 'r');

% 1:frame_id, 2:target_id, 3:target_status, 4:center_x, 5:center_y, 6:width, 7:height, 8:score
C = textscan(fid,'%d %d %d %f %f %f %f %f','HeaderLines', 1);

% image directory
root_dir = '/home/yuxiang/Projects/Tracking/Datasets/VIVID';
image_dir = fullfile(root_dir, seq_idx);
filenames = dir(fullfile(image_dir, '*.jpg'));
num = length(filenames);

% build the IDL structure
idl = struct('bb', cell(num, 1), 'img', cell(num, 1), 'score', cell(num, 1), 'id', cell(num,1));
for i = 1:num
    index = C{1} == i - 1;
    idl(i).bb = [C{4}(index)-C{6}(index)/2 C{5}(index)-C{7}(index)/2 ...
        C{4}(index)+C{6}(index)/2 C{5}(index)+C{7}(index)/2];
    idl(i).id = C{2}(index)';
    
    % default scores
    idl(i).score = C{8}(index);
    
    % image name
    idl(i).img = filenames(i).name;
end