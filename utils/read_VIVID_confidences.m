function [dres, bboxes] = read_VIVID_confidences(seq_idx)

root_dir = '../data/Confidences';

% confidence directory
confidence_dir = fullfile(root_dir, seq_idx);
filenames = dir(fullfile(confidence_dir, '*.conf'));
num = length(filenames);

% build the dres structure
dres.x = [];
dres.y = [];
dres.w = [];
dres.h = [];
dres.r = [];
dres.fr = [];
bboxes = [];
count = 0;

for i = 1:num
	top = load_VIVID_confidence(fullfile(confidence_dir, filenames(i).name));
    if isempty(top) == 0
        bboxes(i).bbox = top(:,1:5);
    else
        bboxes(i).bbox = [];
    end
    % collect bounding boxes
    for j = 1:size(top,1)
        count = count + 1;
        dres.x(count,1) = top(j,1);
        dres.y(count,1) = top(j,2);
        dres.w(count,1) = top(j,3);
        dres.h(count,1) = top(j,4);
        dres.r(count,1) = top(j,5);
        dres.fr(count,1) = i;
    end
end