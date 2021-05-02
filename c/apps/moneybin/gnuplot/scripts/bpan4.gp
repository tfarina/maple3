reset

set title "BPAN4 (Daily)"

set key top

set grid

set yrange [0:14]
set ytics 1 # smaller ytics
set mytics 1

set timefmt "%Y-%m-%d"  # specify our time string format
set xdata time          # tells gnuplot the x axis is time data
set format x "%b %y"

set xtics rotate # rotate labels on the x axis

set terminal png enhanced
set output 'img/bpan4.png'

plot 'data/bpan4.dat' using 1:2:4:3:5 notitle with candlesticks lc rgbcolor "black", \
     'data/bpan4.dat' using 1:8 title '5-day Moving Average' with lines lc rgbcolor "red", \
     'data/bpan4.dat' using 1:9 title '20-day Moving Average' with lines lc rgbcolor "blue"
