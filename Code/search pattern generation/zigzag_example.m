%% zigzag_example

input_wp_filename='zigzag1.txt';

lonfirst=true;

outfilename10='zigzag1_10degrees.csv';
density10=10; %in metres
angle10=10;

outfilename30='zigzag1_30degrees.csv';
density30=20; 
angle30=30;

% start1=[10.32070046368862,55.47399682001517]';
% end1=[10.32578636424596,55.47397658495616]';
% dens1=[10.3211088273502,55.47399689368879]';
% altitude=24; %metres (above sea level if you want absolute coordinates).

zigzag(input_wp_filename,density10,angle10,outfilename10,lonfirst);
zigzag(input_wp_filename,density30,angle30,outfilename30,lonfirst);