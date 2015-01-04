function dres = read_tracking_results(filename)

fid = fopen(filename, 'r');

% frame_id, target_id, target_status, center_x, center_y, width, height, score
C = textscan(fid,'%d %d %d %f %f %f %f %f','HeaderLines', 1);

% build the dres structure
dres.x = C{4} - C{6}/2;
dres.y = C{5} - C{7}/2;
dres.w = C{6};
dres.h = C{7};
dres.r = C{8};
dres.fr = C{1};