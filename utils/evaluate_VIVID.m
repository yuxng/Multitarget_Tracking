function evaluate_VIVID

seq_idx = 'egtest01';

% read annotations in idl format 
gt = read_VIVID_annotations(seq_idx);
people  = sub(gt, find(gt.w < 0));
% read detection results
dres = read_VIVID_confidences(seq_idx);
% read tracking results
dres_track = read_tracking_results('../cache/results.txt');

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
hold off;

figure(2);
draw_PR_curve(1-missr, prec, 'g');
hold off;

names = {'Tracking', 'Detection'};
figure(1);
legend(names);
figure(2);
legend(names);