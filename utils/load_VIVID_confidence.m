function [top, conf] = load_VIVID_confidence(filename)

fp = fopen(filename, 'r');

header = fread(fp, 4, 'char');
if (header(1) ~= 'C' && ...
    header(2) ~= 'O' && ...
    header(3) ~= 'N' && ...
    header(4) ~= '4')
    fprintf('invalid header %s\n', header);
end

top = [];
num_dets = fread(fp, 1, 'uint');
for i = 1:num_dets
    top(i, :) = fread(fp, 5, 'single');
end

if nargout < 2
	fclose(fp);
	conf = {};
	return;
end

num_conf = 1;
for i = 1:num_conf
    % map size
    map_size = fread(fp, 2, 'single')';
    % write map
    conf{i}.map = fread(fp, map_size, 'single');
end

fclose(fp);