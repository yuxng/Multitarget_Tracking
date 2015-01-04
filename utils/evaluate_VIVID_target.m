function evaluate_VIVID_target

seq_idx = 'egtest01';

% read ground truth
idl = read_VIVID_annotations_idl(seq_idx);

% read tracking results
file_tracking = '../cache/results.txt';
trackidl = read_tracking_idl(file_tracking, seq_idx); 

% matching ground truth and tracking results
num = numel(idl);
for i = 1:num
   bbox = idl(i).bb;
   count = size(bbox, 1);
   det = zeros(count, 1);
   idl(i).match = -1 * ones(count,1);
   bbox_pr = trackidl(i).bb;
   trackidl(i).match = -1 *ones(size(bbox_pr,1),1);
   for j = 1:size(bbox_pr,1)
        % compute box overlap
        if isempty(bbox) == 0
            o = box_overlap(bbox, bbox_pr(j,:));
            [maxo, index] = max(o);
            if maxo >= 0.5 && det(index) == 0
                idl(i).match(index) = trackidl(i).id(j);
                trackidl(i).match(j) = idl(i).id(index);
                det(index) = 1;      
            end
        end
   end
end

% compute id
id_gt = [];
id_tr = [];
for i = 1:num
    if isempty(idl(i).id) == 0
        id_gt = [id_gt; idl(i).id];
    end
    if isempty(trackidl(i).id) == 0
        id_tr = [id_tr; trackidl(i).id'];
    end
end
id_gt = unique(id_gt);
id_tr = unique(id_tr);

% build tracklets for ground truth
num_gt = numel(id_gt);
tracklet_gt = struct('id', cell(num_gt, 1));
for i = 1:num_gt
    tracklet_gt(i).id = id_gt(i);
    tracklet_gt(i).frames = [];
    tracklet_gt(i).match = [];
end
for i = 1:num
   for j = 1:numel(idl(i).id)
       index = id_gt == idl(i).id(j);
       tracklet_gt(index).frames(end+1) = i;
       tracklet_gt(index).match(end+1) = idl(i).match(j);
   end
end

% compute measurements
fprintf('GT = %d\n', num_gt);
percentage = zeros(num_gt, 1);
fragments = zeros(num_gt, 1);
for i = 1:num_gt
    num_tracked = numel(find(tracklet_gt(i).match >= 0));
    num_total = numel(tracklet_gt(i).match);
    percentage(i) = num_tracked / num_total;
    fragments(i) = compute_fragments(tracklet_gt(i).match);
    fprintf('Trajectory %d: %f covered, %d fragments\n', i, percentage(i), fragments(i));
end
fprintf('Fragments %d\n', sum(fragments));

mt = numel(find(percentage > 0.8)) / num_gt;
fprintf('Mostly tracked (MT): %f\n', mt);
ml = numel(find(percentage < 0.2)) / num_gt;
fprintf('Mostly lost (ML): %f\n', ml);
fprintf('Partially tracked (PT): %f\n', 1-mt-ml);

% build tracklets for tracking results
num_tr = numel(id_tr);
tracklet_tr = struct('id', cell(num_tr, 1));
for i = 1:num_tr
    tracklet_tr(i).id = id_tr(i);
    tracklet_tr(i).frames = [];
    tracklet_tr(i).match = [];
end
for i = 1:num
   for j = 1:numel(trackidl(i).id)
       index = id_tr == trackidl(i).id(j);
       tracklet_tr(index).frames(end+1) = i;
       tracklet_tr(index).match(end+1) = trackidl(i).match(j);
   end
end

% compute id switches
id_switches = 0;
for i = 1:num_tr
    id_switches = id_switches + compute_id_switches(tracklet_tr(i).match);
end
fprintf('id switches %d\n', id_switches);

% fragments
function num = compute_fragments(match)

num = 0;
start = 0;
id = -1;
for i = 1:numel(match)
    if start == 0 && match(i) >= 0
        start = 1;
        id = match(i);
        num = num + 1;
    end
    if start == 1 && (match(i) < 0 || match(i) ~= id)
        start = 0;
    end
end
if num ~= 0
    num = num - 1;
end

% ID switches
function num = compute_id_switches(match)

num = 0;
id = -1;
for i = 1:numel(match)
    if id == -1 && match(i) >= 0
        id = match(i);
    end
    if id ~= -1 && match(i) >= 0
        if id ~= match(i)
            num = num + 1;
            id = match(i);
        end
    end
end