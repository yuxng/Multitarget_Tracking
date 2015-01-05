function show_VIVID_detections_csv

root_dir = '../data/VIVID';
detection_dir = '../data/Detections';
snames = {'egtest01', 'egtest02', 'egtest03', 'egtest04'};
filter_scales = [0.935, 0.85, 0.85, 0.85];
filter_size = 40;
threshold = 4;

figure(1);
for seq_idx = 1:numel(snames)
    seq_name = snames{seq_idx};
    fprintf('%s\n', seq_name);
    
    % read the detection results
    filename = fullfile(detection_dir, seq_name, 'correlation_detections.csv');
    fid = fopen(filename, 'r');
    C = textscan(fid, '%d %s %f %f %f');
    fclose(fid);
    frame_ids = C{1};
    frame_names = C{2};
    width = filter_size / filter_scales(seq_idx);
    height = width;
    dets = [C{3}-width/2 C{4}-height/2 C{3}+width/2 C{4}+height/2 C{5}];
    
    % find the number of frames
    files = dir(fullfile(root_dir, seq_name, '*.jpg'));
    N = numel(files);
    
    % for each frame
    for i = 1:N
        fprintf('Frame %s\n', files(i).name);
        % find index
        index = strcmp(files(i).name, frame_names) == 1;
        
        % get the detections
        bbox = dets(index, :);
        index = nms(bbox, 0.3);
        bbox = bbox(index,:);        
        num = size(bbox, 1);
        
        % show bounding boxes
        filename = fullfile(root_dir, seq_name, files(i).name);
        I = imread(filename);
        imshow(I);
        hold on;
        for j = 1:num
            if bbox(j,5) < threshold
                continue;
            end
            bbox_draw = [bbox(j,1) bbox(j,2) bbox(j,3)-bbox(j,1) bbox(j,4)-bbox(j,2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 4);
            s = sprintf('%.2f', bbox(j,5));
            text(bbox_draw(1), bbox_draw(2), s, 'FontSize', 8, 'BackgroundColor', 'c');            
        end
        hold off;
        
        pause;
    end
end