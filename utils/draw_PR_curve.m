function draw_PR_curve(recall, precision, col, pattern)

if nargin < 3
	col = 'b';
	pattern = '-';
elseif nargin < 4
	pattern = '-';
end

plot(recall, precision, pattern, 'color', col, 'LineWidth',3);
h = xlabel('Recall');
set(h,'FontSize',20);
h = ylabel('Precision');
set(h,'FontSize',20);
set(gca, 'fontsize', 15);