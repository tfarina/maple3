reset

set title "AGEAS Stock Chart"

set key bottom left

set grid

set yrange [24:56]
set ytics 2             # smaller ytics

set timefmt "%Y-%m-%d"  # specify our time string format
set xdata time          # tells gnuplot the x axis is time data
set format x "%b %y"

set xtics rotate        # rotate labels on the x axis

set terminal png enhanced
set output 'img/ageas.png'

plot 'data/ageas.dat' using 1:5 notitle with lines lc rgbcolor "black", \
     'data/ageas.dat' using 1:8 title '5-day Moving Average' with lines lc rgbcolor "green", \
     'data/ageas.dat' using 1:9 title '20-day Moving Average' with lines lc rgbcolor "red"
