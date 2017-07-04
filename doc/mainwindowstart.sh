#@	{
#@	"targets":
#@		[{
#@		"name":"mainwindowstart.png","dependencies":
#@			[{"ref":"Xvfb","rel":"tool"}
#@			,{"ref":"import","rel":"tool"}
#@			,{"ref":"catia","rel":"tool"}
#@			,{"ref":"../anja","rel":"misc"}]
#@		},{
#@		"name":"anja_layout.txt","dependencies":
#@			[{"ref":"Xvfb","rel":"tool"}
#@			,{"ref":"import","rel":"tool"}
#@			,{"ref":"../anja","rel":"misc"}]
#@		},{
#@		"name":"anja_jackports.txt","dependencies":
#@			[{"ref":"Xvfb","rel":"tool"}
#@			,{"ref":"import","rel":"tool"}
#@			,{"ref":"../anja","rel":"misc"}]
#@		}]
#@	}

set -e

target_dir=$1
in_dir=$2
Xvfb :5 -screen 0 1366x768x24 -fbdir /dev/shm &
server=$!
export DISPLAY=:5
while ! xdpyinfo >/dev/null 2>&1; do
   sleep 0.50s
done
export JACK_DEFAULT_SERVER=dummy
jackd --no-mlock -p 64 --no-realtime -d dummy -p 4096 &
jack=$!
jack_wait -w --server $JACK_DEFAULT_SERVER
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT INT TERM HUP
mkfifo "$tmpdir/anja_fifo"
"$target_dir"/anja --script="$tmpdir/anja_fifo" > "$target_dir"/"$in_dir"/anja_layout.txt &
anja=$!
while ! jack_lsp | grep anja >/dev/null 2>&1; do
	sleep 0.1s
done
jack_lsp | grep '\.anja' > "$target_dir"/"$in_dir"/anja_jackports.txt
anjawin=$(xdotool search --any --onlyvisible --pid $anja)
import -window $anjawin "$target_dir"/"$in_dir"/mainwindowstart.png
echo "layout inspect
exit" > "$tmpdir/anja_fifo"
wait $anja
kill $jack
kill $server
