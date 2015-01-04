function show_VIVID_detections

threshold = 255*0.5;
width = 40;
height = 40;
root_dir = '../data/VIVID';
detection_dir = '../data/Detections';
snames = {'egtest01', 'egtest02', 'egtest03', 'egtest04'};
heatmap_scales = [0.4672, 0.4250, 0.4250, 0.4250];

for seq_idx = 1:numel(snames)
    seq_name = snames{seq_idx};
    scale = heatmap_scales(seq_idx);
    fprintf('%s\n', seq_name);
    
    % find the number of frames
    files = dir(fullfile(root_dir, seq_name, '*.jpg'));
    N = numel(files);
    
    % for each frame
    for i = 1:N
        fprintf('Frame %s\n', files(i).name);
        % heatmap
        filename = fullfile(detection_dir, seq_name, 'correlation_maps', ...
            sprintf('CORR_HEAT_MAP_%06d.jpg', i-1));
        heatmap = imread(filename);
        
        % get the detections
        index = find(heatmap > threshold);
        [row, col] = ind2sub(size(heatmap), index);
        bbox = [col/scale-width/2 row/scale-height/2 col/scale+width/2 row/scale+height/2 double(heatmap(index))];
        index = nms(bbox, 0.5);
        bbox = bbox(index,:);
        
        % meanshift clustering on bounding box centers
%         x = [(bbox(:,1)+bbox(:,3))/2 (bbox(:,2)+bbox(:,4))/2]';
%         bandwidth = width;
%         C = MeanShiftCluster(x, bandwidth)';
%         bbox = [C(:,1)-width/2 C(:,2)-height/2 C(:,1)+width/2 C(:,2)+height/2];
        num = size(bbox, 1);
        
        % show bounding boxes
        filename = fullfile(root_dir, seq_name, files(i).name);
        I = imread(filename);
        imshow(I);
        hold on;
        for j = 1:num
            bbox_draw = [bbox(j,1) bbox(j,2) bbox(j,3)-bbox(j,1) bbox(j,4)-bbox(j,2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g');
        end
        hold off;
        pause;
    end
end