import matplotlib
matplotlib.use('Agg')


def test_sas():
    import script.ml.detection.sas.sas as sas
    sas.main()


def test_umbellula():
    import script.ml.detection.umbellula.umbellula as umbellula
    umbellula.main()
