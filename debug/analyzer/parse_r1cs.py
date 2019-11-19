#!/usr/bin/env python3

import json
import os

"""
Finds the index corresponding to `annotation`
in the annotation set (which has the structure below)

"variables_annotations":
    [
        {
            "index": INDEX,
            "annotation": ANNOTATION
        },

        ...

        {
            "index": INDEX,
            "annotation": ANNOTATION
        }
    ]
"""
def get_index(annotation_set, annotation):
    for i in range(len(annotation_set)):
        if annotation_set[i]["annotation"] == annotation:
            return annotation_set[i]["index"]


"""
Returns the set of constraints in which a given wire figures
(wire = index of a variable annotation)
The set of constraints has the structure below:


"constraints":
    [
        {
            "constraint_id": ID,
            "constraint_annotation": ANNOTATION
            "linear_combination": {
                "A":
                    [
                        {
                            "index": "0", # Index of the wire/variable
                            "value": "0x1" # Scalar by which the wire value is multiplied by in the linear combination A
                        }
                    ],
                "B":
                    [
                        {
                            "index": "530",
                            "scalar": "0x1"
                        },
                        {
                            "index": "531",
                            "scalar": "0x2"
                        },
                        {
                            "index": "532",
                            "scalar": "0x4"
                        }
                    ],
                "C":
                    [
                        {
                            "index": "2",
                            "scalar": "0x1"
                        }
                    ]
            }
        }
    ]
"""
#def get_constraints(constraints_set, annotation_index):
#    # Array of ID of the constraints using the provided annotation index
#    constraints_id = []
#    for i in range(len(constraints_set)):
#        if constraints_set[i][]


if __name__ == "__main__":
    path_zeth = os.environ["ZETH_DEBUG_DIR"]
    filename = "r1cs.json"

    # Read file
    file_path = os.path.join(path_zeth, filename)
    with open(file_path, 'r') as r1cs_file:
        data=r1cs_file.read()

    # Parse file
    r1cs_obj = json.loads(data)
    print("R1CS succesfully loaded")

    nb_vars = r1cs_obj["num_variables"]
    print("Number of variables: ", nb_vars)
    nb_const = r1cs_obj["num_constraints"]
    print("Number of constraints: ", nb_const)

    print("Display the first 20 entries of the variables annotations")
    var_annotations = r1cs_obj["variables_annotations"]
    for i in range(0,50):
        print("Annotation: " + str(var_annotations[i]))

    # Check the 31th bit of phi
    annotation_to_check = "joinsplit_gadget phi bits_31"
    # Return the constraints in which this variable/wire appears
    # Index corresponding to the annotation
    annotation_code = get_index(var_annotations, annotation_to_check)
    print("Index of the annotation to check: ", annotation_code)
