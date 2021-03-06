"""Compute boolean operations between blobby and eight."""

import SVMTK as svm
from pathlib import Path


def print_stats(surf: svm.Surface) -> None:
    """Print num_facet, num_edges and num vertices."""
    print("num_faces: {}".format(surf.num_faces()) )
    print("num_edges: {}".format(surf.num_edges()) )
    print("num_vertices: {}".format(surf.num_vertices()))


class BooleanOperation:
    """Class for performing boolean operation on surffaces.

    Operations include intersection, union and difference.
    """

    def __init__(self, input_name1: str, input_name2: str) -> None:
        """Load surfaces from filepaths."""
        self.surf1 = svm.Surface(input_name1)
        self.surf2 = svm.Surface(input_name2)

    def intersection(self) -> None:
        """Copute the intersection between surface1 and surface2."""
        self.surf1.intersection(self.surf2)

    def union(self) -> None:
        """Copute the union between surface1 and surface2."""
        self.surf1.union(self.surf2)

    def difference(self) -> None:
        """Copute the difference between surface1 and surface2."""
        self.surf1.difference(self.surf2)

    def save(self, output_name: str) -> None:
        """Save surface1 as `output_name`."""
        self.surf1.save(str(output_name))       # NB! Must convert to string for pybind11


if __name__ == "__main__":
    outdir = Path("results")
    outdir.mkdir(exist_ok=True)

    bop = BooleanOperation(
        "../Data/blobby.off",
        "../Data/eight.off"
    )

    print_stats(bop.surf1)
    bop.difference()
    bop.save(outdir/"blobby_difference_eight.off")

    # Operations are performed in-place on surface1
    bop = BooleanOperation(
        "../Data/blobby.off",
        "../Data/eight.off"
    )

    bop.union()
    bop.save(outdir/"blobby_union_eight.off")

    # 
    bop = BooleanOperation(
        "../Data/blobby.off",
        "../Data/eight.off"
    )

    bop.intersection()
    bop.save(outdir/"blobby_intersection_eight.off")
