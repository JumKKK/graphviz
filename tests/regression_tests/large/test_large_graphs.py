"""
Tests of large and/or expensive graphs.
"""

import os
from pathlib import Path
import platform
import subprocess
import sys
import pytest

sys.path.append(os.path.join(os.path.dirname(__file__), "../../../rtest"))
from gvtest import dot #pylint: disable=wrong-import-position

@pytest.mark.skipif(platform.system() == "Windows" and
                    platform.architecture()[0] == "32bit",
                    reason="https://gitlab.com/graphviz/graphviz/-/issues/1710")
def test_long_chain():
  """
  This test will fail on 32bit Windows machine if compiled with stack size < 16MB.
  long_chain input file generated using code below:
    from graphviz import Digraph

    graph = Digraph(format="svg")

    prev = "start"
    graph.node("start", label="start")

    for i in range(33000):
      new_node = str(i)
      graph.node(new_node, label=new_node, shape="rectangle")

      graph.edge(prev, new_node)

      prev = new_node

    graph.render("long_chain")
  """
  input = Path(__file__).parent / "long_chain"
  subprocess.check_call([
    "dot", "-Tsvg", "-O", input
  ])

def test_wide_clusters():
  """
  A simple regression test for https://gitlab.com/graphviz/graphviz/-/issues/2080#
  """
  dot("svg", Path(__file__).parent / "wide_clusters")
