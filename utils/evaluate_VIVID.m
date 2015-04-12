function evaluate_VIVID

seq_idx = 'egtest01';
append = 'simple';

% read annotations in idl format 
gt = read_VIVID_annotations(seq_idx);
people  = sub(gt, find(gt.w < 0));
% read detection results
% dres = read_VIVID_confidences(seq_idx);
dres = read_VIVID_detections(seq_idx);
% read tracking results
% file_tracking = sprintf('../cache/%s_%s_results.txt', seq_idx, append);
file_tracking = '../cache/results.txt';
dres_track = read_tracking_results(file_tracking);

% compute fppi, recall and precision
[missr, fppi, prec] = score(dres_track, gt, people);

figure(1);
draw_fppimr(1-missr, fppi, 'r');
hold on;

figure(2);
draw_PR_curve(1-missr, prec, 'r');
hold on;

[missr, fppi, prec] = score(dres, gt, people);

figure(1);
draw_fppimr(1-missr, fppi, 'g');
title(seq_idx);
hold off;

figure(2);
draw_PR_curve(1-missr, prec, 'g');
title(seq_idx);
hold off;

names = {'Tracking', 'Detection'};
figure(1);
legend(names);
figure(2);
legend(names);