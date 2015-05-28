%% spiral_example
input_wp_filename='zigzag1.txt';

lonfirst=true;

outfilename30='spiral1_30m.csv';
outfilename50='spiral1_50m.csv';
outfilename100i='spiral1_100m_inwards.csv';

spiral(input_wp_filename,30,outfilename30,lonfirst,false);
spiral(input_wp_filename,50,outfilename50,lonfirst,false);
spiral(input_wp_filename,100,outfilename100i,lonfirst,true);