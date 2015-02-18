function evaluate_VIVID_multiple

seq_idx = 'egtest01';

% read annotations in idl format 
gt = read_VIVID_annotations(seq_idx);
people  = sub(gt, find(gt.w < 0));

figure(1);
colors = {'r', 'g', 'b', 'k', 'c'};
count = 0;
hold on;
for err = [0 5 78 10 15]

    file_tracking = sprintf('../cache/results_error/results_mean_%d.txt', err);
    dres_track = read_tracking_results(file_tracking);

    % compute fppi, recall and precision
    [missr, fppi, prec] = score(dres_track, gt, people);
    count = count + 1;
    draw_PR_curve(1-missr, prec, colors{count});
end

legend('noise free', '5 pixel std', '7.5 pixel std', '10 pixel std', '15 pixel std');
title('noise analysis in target location on egtest01');