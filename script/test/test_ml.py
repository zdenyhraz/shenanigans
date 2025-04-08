import matplotlib
matplotlib.use('Agg')


def test_sas_segm():
    import script.ml.detection.sas.sas_segm as sas_segm
    sas_segm.main()


def test_umbellula():
    import script.ml.detection.umbellula.umbellula as umbellula
    umbellula.main()
