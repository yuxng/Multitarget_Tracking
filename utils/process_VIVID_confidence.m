function process_VIVID_confidence

root_dir = '../data/VIVID';
detection_dir = '../data/Detections';
conf_dir = '../data/Confidences';
snames = {'egtest01', 'egtest02', 'egtest03', 'egtest04'};
heatmap_scales = [2.1405, 2.3529, 2.3529, 2.3529];
width = 40;
height = 40;

for seq_idx = 1:numel(snames)
    seq_name = snames{seq_idx};
    fprintf('%s\n', seq_name);
    
    % find the number of frames
    files = dir(fullfile(root_dir, seq_name, '*.jpg'));
    N = numel(files);
    
    % read detections
    filename = fullfile(detection_dir, seq_name, 'correlation_detections.csv');
    fid = fopen(filename, 'r');
    % skip the first line
    % header = fgets(fid);
    % read the following
    C = textscan(fid, '%d %s %f %f %f');
    
    % for each frame
    for i = 1:N
        fprintf('Frame %d\n', i);
        id = i - 1;
        % detection
        index = find(C{1} == id);
        if isempty(index) == 1
            det = [];
        else
            % det = [C{3}(index)-width/2 C{4}(index)-height/2 width*ones(numel(index),1) height*ones(numel(index),1)];
            det = [C{3}(index)-width/2 C{4}(index)-height/2 C{3}(index)+width/2 C{4}(index)+height/2 C{5}(index)];
            I = nms(det, 0.3);
            det = det(I,:);
            det(:,3) = width;
            det(:,4) = height;
            det(:,5) = [];
        end
        % heatmap
        filename = fullfile(detection_dir, seq_name, 'correlation_maps', ...
            sprintf('CORR_HEAT_MAP_%06d.jpg', id));
        if exist(filename, 'file')
            conf = imread(filename);
            % normalization
            conf = double(conf) / 255.0 - 0.5;
            % assign confidence score to the detection
            num = size(det, 1);
            scores = zeros(num, 1);
            for j = 1:num
                cx = floor((det(j,1) + det(j,3)/2) / heatmap_scales(seq_idx));
                cy = floor((det(j,2) + det(j,4)/2) / heatmap_scales(seq_idx));
                if cx <= 0 || cx > size(conf, 2) || cy <= 0 || cy > size(conf, 1)
                    scores(j) = -1;
                else
                    scores(j) = double(conf(cy, cx));
                end
            end
            det = [det scores];
        else
            conf = [];
        end
        % save confidence file
        dirname = fullfile(conf_dir, seq_name);
        if exist(dirname, 'dir') == 0
            mkdir(dirname);
        end
        filename = fullfile(dirname, sprintf('Frame_%06d.conf', id));
        save_VIVID_confidence(filename, det, conf);  
    end
end

function save_VIVID_confidence(filename, det, conf)

fp = fopen(filename, 'w');

fwrite(fp,'CON4','char');

num_dets = size(det, 1);
fwrite(fp, num_dets, 'uint');
for i = 1:num_dets
    fwrite(fp, single(det(i, :)), 'single');
end

% map size
map_size = size(conf);
fwrite(fp, single(map_size), 'int');
% write map
fwrite(fp, single(conf), 'single');

fclose(fp);