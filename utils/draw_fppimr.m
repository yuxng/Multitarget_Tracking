function draw_fppimr(recall, fppi, col, pattern)

if nargin < 3
	col = 'b';
	pattern = '-';
elseif nargin < 4
	pattern = '-';
end

plot(fppi, 1-recall, pattern, 'color', col, 'linewidth', 2);
set(gca, 'xscale', 'log');
set(gca, 'yscale', 'log');
axis([0.002 10 0.2 1]);
set(gca, 'YTick', 0.2:0.1:1);
set(gca, 'fontsize', 15);

% set(gca, 'XTick', 0.003:10);
grid on
h = xlabel('Fppi');
set(h, 'fontsize', 20);
h = ylabel('Miss rate');
set(h, 'fontsize', 20);

end
