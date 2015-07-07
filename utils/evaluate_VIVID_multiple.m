function evaluate_VIVID_multiple

seq_idx = 'egtest01';

% read annotations in idl format 
gt = read_VIVID_annotations(seq_idx);
people  = sub(gt, find(gt.w < 0));

% figure(1);
% colors = {'r', 'g', 'b', 'k', 'c', 'y'};
% count = 0;
% hold on;
% for err = 0:5
% 
%     file_tracking = sprintf('../cache/results_off_%d.txt', err);
%     dres_track = read_tracking_results(file_tracking);
% 
%     % compute fppi, recall and precision
%     [missr, fppi, prec] = score(dres_track, gt, people);
%     count = count + 1;
%     draw_PR_curve(1-missr, prec, colors{count});
% end
% 
% % legend('noise free', '2.5 pixel std', '5 pixel std', '7.5 pixel std', '10 pixel std', '15 pixel std');
% legend('noise free', '2.5 pixel offset', '5 pixel offset', '7.5 pixel offset', '10 pixel offset', '15 pixel offset');
% % title('Gaussian noise analysis in target location on egtest01');
% title('Constant Offset analysis in target location on egtest01');
close all;
file_tracking = sprintf('../cache/results_%s_full_1.txt', seq_idx);
dres_track = read_tracking_results(file_tracking);
[missr, fppi, prec] = score(dres_track, gt, people);
draw_PR_curve(1-missr, prec, 'r');
hold on;

% file_tracking = sprintf('../cache/results_%s_simple.txt', seq_idx);
file_tracking = '../cache/results_std_0.txt';
dres_track = read_tracking_results(file_tracking);
[missr, fppi, prec] = score(dres_track, gt, people);
ylim([0,1]);
draw_PR_curve(1-missr, prec, 'g');
legend('Original', 'Simplified');