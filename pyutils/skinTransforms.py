# -*- coding: utf-8 -*-
"""This module is mean to be used to get the main training data for train the model to be used on ml_rivets.mll node
This code is to be used on maya with numpy library

MIT License

Copyright (c) 2020 Mauro Lopez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""
import meshData
import numpy as np
from maya import cmds


def closestTriangleToTransform(transform, meshName):
    """get the closest mesh triangles ids from a transomr position

    Args:
        transform (str): name of the transfomr to get the position
        meshName (str): name of the mesh to get the triangle

    Returns:
        list: 3 mesh vertices ids
    """
    faceVertices, points = meshData.getMeshData(meshName)
    vertexFaces = meshData.getMeshVertexFaces(faceVertices)
    point = np.array(cmds.xform(transform, q=1, ws=1, t=1), dtype=np.double)
    return meshData.getClosestTriangle(point, points, vertexFaces, faceVertices)


def createSkinTansformNode(skinCluster, transforms):
    """get the skinweights form the skincluster, create a skintransforms node
    and connect it to drive the transforms

    Args:
        skinCluster (str): name of the skincluster node
        transforms (list): list of transforms names
    Returns:
        str: name of the created skinTransforms node
    """
    node = cmds.createNode('skinTransforms')
    influences = cmds.listConnections('{}.matrix'.format(skinCluster), s=1, d=0)
    for i, jnt in enumerate(influences):
        cmds.connectAttr('{}.worldMatrix[0]'.format(jnt), '{}.matrix[{}]'.format(node,i))
        m = cmds.getAttr('{}.wim[0]'.format(jnt))
        cmds.setAttr ('{}.bindPreMatrix[{}]'.format(node, i), m, type="matrix")
    mesh = cmds.deformer(skinCluster, q=1, g=1)[0]
    for i, each in enumerate(transforms):
        triangle = closestTriangleToTransform(each, mesh)
        weights=list()
        positions=list()
        for vtx in triangle:
            vtxName = '{}.vtx[{}]'.format(mesh, vtx)
            weights.extend(cmds.skinPercent(skinCluster, vtxName, query=True, value=True ))
            positions.extend(cmds.xform(vtxName, q=1, ws=1, t=1))
        cmds.setAttr('{}.weightsList[{}].weights'.format(node, i), weights, type="doubleArray")
        cmds.setAttr('{}.weightsList[{}].points'.format(node, i), positions, type="doubleArray")
        m = cmds.getAttr('{}.wm[0]'.format(each));
        cmds.setAttr('{}.weightsList[{}].pbm'.format(node, i), m, type="matrix")
    for i, loc in enumerate(transforms):
        dec = cmds.createNode('decomposeMatrix')
        cmds.connectAttr('{}.outputs[{}]'.format(node, i), '{}.inputMatrix'.format(dec))
        cmds.connectAttr('{}.outputTranslate'.format(dec), '{}.translate'.format(loc))
        cmds.connectAttr('{}.outputRotate'.format(dec), '{}.rotate'.format(loc))
        cmds.connectAttr('{}.outputScale'.format(dec), '{}.scale'.format(loc))
    return node
