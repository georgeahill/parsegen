from io import StringIO
from railroad import Diagram, Choice, NonTerminal, Optional, Sequence, Skip
import sys
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPM

# d = Diagram(Choice(0, Sequence("a", NonTerminal("S"), "b"), Sequence("b", "a"), Skip()), type="complex")
d = Diagram(Choice(0, Sequence("a", NonTerminal("S"), "b"), Sequence("b", "a"), Skip()), type="complex")


svg_code = StringIO()
d.writeSvg(sys.stdout.write)
