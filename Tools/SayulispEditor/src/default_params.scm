;; Generates Engine.
(define my-engine (gen-engine))

;; Configures Engine.
(my-engine (quote @material) (quote (0 100 400 400 600 1200 1000000)))
(my-engine (quote @enable-quiesce-search) (quote #t))
(my-engine (quote @enable-repetition-check) (quote #t))
(my-engine (quote @enable-check-extension) (quote #t))
(my-engine (quote @ybwc-limit-depth) (quote 4))
(my-engine (quote @ybwc-invalid-moves) (quote 3))
(my-engine (quote @enable-aspiration-windows) (quote #t))
(my-engine (quote @aspiration-windows-limit-depth) (quote 5))
(my-engine (quote @aspiration-windows-delta) (quote 15))
(my-engine (quote @enable-see) (quote #t))
(my-engine (quote @enable-history) (quote #t))
(my-engine (quote @enable-killer) (quote #t))
(my-engine (quote @enable-hash-table) (quote #t))
(my-engine (quote @enable-iid) (quote #t))
(my-engine (quote @iid-limit-depth) (quote 5))
(my-engine (quote @iid-search-depth) (quote 4))
(my-engine (quote @enable-nmr) (quote #t))
(my-engine (quote @nmr-limit-depth) (quote 4))
(my-engine (quote @nmr-search-reduction) (quote 4))
(my-engine (quote @nmr-reduction) (quote 3))
(my-engine (quote @enable-probcut) (quote #f))
(my-engine (quote @probcut-limit-depth) (quote 4))
(my-engine (quote @probcut-margin) (quote 400))
(my-engine (quote @probcut-search-reduction) (quote 3))
(my-engine (quote @enable-history-pruning) (quote #f))
(my-engine (quote @history-pruning-limit-depth) (quote 4))
(my-engine (quote @history-pruning-move-threshold) (quote 0.6))
(my-engine (quote @history-pruning-invalid-moves) (quote 10))
(my-engine (quote @history-pruning-threshold) (quote 0.5))
(my-engine (quote @history-pruning-reduction) (quote 1))
(my-engine (quote @enable-lmr) (quote #t))
(my-engine (quote @lmr-limit-depth) (quote 4))
(my-engine (quote @lmr-move-threshold) (quote 0.3))
(my-engine (quote @lmr-invalid-moves) (quote 4))
(my-engine (quote @lmr-search-reduction) (quote 1))
(my-engine (quote @enable-futility-pruning) (quote #t))
(my-engine (quote @futility-pruning-depth) (quote 3))
(my-engine (quote @futility-pruning-margin) (quote 400))
(my-engine (quote @pawn-square-table-opening) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 10 15 20 20 15 10 5 10 20 30 40 40 30 20 10 15 30 45 60 60 45 30 15 20 40 60 80 80 60 40 20 25 50 75 100 100 75 50 25 30 60 90 120 120 90 60 30)))
(my-engine (quote @knight-square-table-opening) (quote (-30 -20 -10 0 0 -10 -20 -30 -20 -10 0 10 10 0 -10 -20 -10 0 10 20 20 10 0 -10 0 10 20 30 30 20 10 0 10 20 30 40 40 30 20 10 20 30 40 50 50 40 30 20 10 20 30 40 40 30 20 10 0 10 20 30 30 20 10 0)))
(my-engine (quote @bishop-square-table-opening) (quote (15 10 5 0 0 5 10 15 10 20 15 10 10 15 20 10 5 15 20 15 15 20 15 5 5 10 15 25 25 15 10 5 0 7.5 12.5 22.5 22.5 12.5 7.5 0 2.5 7.5 17.5 12.5 12.5 17.5 7.5 2.5 2.5 12.5 7.5 2.5 2.5 7.5 12.5 2.5 7.5 2.5 2.5 0 0 2.5 2.5 7.5)))
(my-engine (quote @rook-square-table-opening) (quote (0 5 10 15 15 10 5 0 0 5 10 15 15 10 5 0 0 5 10 15 15 10 5 0 0 5 10 15 15 10 5 0 0 5 10 15 15 10 5 0 0 5 10 15 15 10 5 0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20)))
(my-engine (quote @queen-square-table-opening) (quote (-5 -5 -5 -5 -5 -5 -5 -5 -5 0 0 0 0 0 0 -5 -5 0 5 5 5 5 0 -5 -5 0 5 10 10 5 0 -5 -5 0 5 10 10 5 0 -5 -5 0 5 5 5 5 0 -5 -5 0 0 0 0 0 0 -5 -5 -5 -5 -5 -5 -5 -5 -5)))
(my-engine (quote @king-square-table-opening) (quote (30 30 30 -30 -30 30 30 30 0 0 0 -30 -30 0 0 0 -30 -30 -40 -40 -40 -40 -30 -30 -30 -40 -40 -40 -40 -40 -40 -30 -50 -50 -50 -50 -50 -50 -50 -50 -60 -60 -60 -60 -60 -60 -60 -60 -70 -70 -70 -70 -70 -70 -70 -70 -80 -80 -80 -80 -80 -80 -80 -80)))
(my-engine (quote @pawn-square-table-ending) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 20 20 20 20 20 20 20 20 40 40 40 40 40 40 40 40 60 60 60 60 60 60 60 60 80 80 80 80 80 80 80 80 100 100 100 100 100 100 100 100 120 120 120 120 120 120 120 120)))
(my-engine (quote @knight-square-table-ending) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)))
(my-engine (quote @bishop-square-table-ending) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)))
(my-engine (quote @rook-square-table-ending) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)))
(my-engine (quote @queen-square-table-ending) (quote (0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)))
(my-engine (quote @king-square-table-ending) (quote (-30 -15 0 15 15 0 -15 -30 -15 0 15 30 30 15 0 -15 0 15 30 45 45 30 15 0 15 30 45 60 60 45 30 15 15 30 45 60 60 45 30 15 0 15 30 45 45 30 15 0 -15 0 15 30 30 15 0 -15 -30 -15 0 15 15 0 -15 -30)))
(my-engine (quote @pawn-attack-table) (quote (0 10 12 14 16 18 20)))
(my-engine (quote @knight-attack-table) (quote (0 8 10 12 14 16 18)))
(my-engine (quote @bishop-attack-table) (quote (0 6 8 10 12 14 16)))
(my-engine (quote @rook-attack-table) (quote (0 4 6 8 10 12 14)))
(my-engine (quote @queen-attack-table) (quote (0 2 4 6 8 10 12)))
(my-engine (quote @king-attack-table) (quote (0 0 0 0 0 0 0)))
(my-engine (quote @pawn-defense-table) (quote (0 10 0 0 0 0 0)))
(my-engine (quote @knight-defense-table) (quote (0 7.5 0 0 0 0 0)))
(my-engine (quote @bishop-defense-table) (quote (0 0 0 0 0 0 0)))
(my-engine (quote @rook-defense-table) (quote (0 5 0 0 0 0 0)))
(my-engine (quote @queen-defense-table) (quote (0 2.5 0 0 0 0 0)))
(my-engine (quote @king-defense-table) (quote (0 10 0 0 0 0 0)))
(my-engine (quote @bishop-pin-table) (quote ((0 0 0 0 0 0 0) (0 0 0 0 5 10 10) (0 0 0 0 20 30 40) (0 0 0 0 0 0 0) (0 0 0 0 100 100 100) (0 0 0 0 100 0 500) (0 0 0 0 100 500 0))))
(my-engine (quote @rook-pin-table) (quote ((0 0 0 0 0 0 0) (0 0 0 0 0 5 10) (0 0 0 0 0 10 20) (0 0 0 0 0 10 20) (0 0 0 0 0 0 0) (0 0 0 0 0 0 250) (0 0 0 0 0 250 0))))
(my-engine (quote @queen-pin-table) (quote ((0 0 0 0 0 0 0) (0 0 0 0 0 0 10) (0 0 0 0 0 0 10) (0 0 0 0 0 0 10) (0 0 0 0 0 0 10) (0 0 0 0 0 0 0) (0 0 0 0 0 0 0))))
(my-engine (quote @pawn-shield-table) (quote (0 0 0 0 0 0 0 0 30 30 30 30 30 30 30 30 0 0 0 0 0 0 0 0 -30 -30 -30 -30 -30 -30 -30 -30 -60 -60 -60 -60 -60 -60 -60 -60 -90 -90 -90 -90 -90 -90 -90 -90 -60 -60 -60 -60 -60 -60 -60 -60 -30 -30 -30 -30 -30 -30 -30 -30)))
(my-engine (quote @weight-pawn-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-knight-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-bishop-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-rook-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-queen-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-king-opening-position) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-pawn-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-knight-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-bishop-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-rook-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-queen-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-king-ending-position) (quote ((2 1) (14 1) (32 0))))
(my-engine (quote @weight-pawn-mobility) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-knight-mobility) (quote ((2 2) (14 2) (32 1))))
(my-engine (quote @weight-bishop-mobility) (quote ((2 3) (14 3) (32 1.5))))
(my-engine (quote @weight-rook-mobility) (quote ((2 2) (14 2) (32 1))))
(my-engine (quote @weight-queen-mobility) (quote ((2 2) (14 2) (32 1))))
(my-engine (quote @weight-king-mobility) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-pawn-center-control) (quote ((2 0) (14 0) (32 5))))
(my-engine (quote @weight-knight-center-control) (quote ((2 0) (14 0) (32 4))))
(my-engine (quote @weight-bishop-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-rook-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-queen-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-king-center-control) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-pawn-sweet-center-control) (quote ((2 0) (14 0) (32 5))))
(my-engine (quote @weight-knight-sweet-center-control) (quote ((2 0) (14 0) (32 4))))
(my-engine (quote @weight-bishop-sweet-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-rook-sweet-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-queen-sweet-center-control) (quote ((2 0) (14 0) (32 2.5))))
(my-engine (quote @weight-king-sweet-center-control) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-pawn-development) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-knight-development) (quote ((2 0) (14 0) (32 15))))
(my-engine (quote @weight-bishop-development) (quote ((2 0) (14 0) (32 15))))
(my-engine (quote @weight-rook-development) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-queen-development) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-king-development) (quote ((2 0) (14 0) (32 0))))
(my-engine (quote @weight-pawn-attack) (quote ((2 0.3) (14 0.3) (32 1))))
(my-engine (quote @weight-knight-attack) (quote ((2 0.3) (14 0.3) (32 1))))
(my-engine (quote @weight-bishop-attack) (quote ((2 0.3) (14 0.3) (32 1))))
(my-engine (quote @weight-rook-attack) (quote ((2 0.3) (14 0.3) (32 1))))
(my-engine (quote @weight-queen-attack) (quote ((2 0.3) (14 0.3) (32 1))))
(my-engine (quote @weight-king-attack) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-pawn-defense) (quote ((2 0.5) (14 0.5) (32 1))))
(my-engine (quote @weight-knight-defense) (quote ((2 0.5) (14 0.5) (32 1))))
(my-engine (quote @weight-bishop-defense) (quote ((2 0.5) (14 0.5) (32 1))))
(my-engine (quote @weight-rook-defense) (quote ((2 0.5) (14 0.5) (32 1))))
(my-engine (quote @weight-queen-defense) (quote ((2 0.5) (14 0.5) (32 1))))
(my-engine (quote @weight-king-defense) (quote ((2 3) (14 3) (32 1))))
(my-engine (quote @weight-bishop-pin) (quote ((2 1) (14 1) (32 1))))
(my-engine (quote @weight-rook-pin) (quote ((2 1) (14 1) (32 1))))
(my-engine (quote @weight-queen-pin) (quote ((2 1) (14 1) (32 1))))
(my-engine (quote @weight-pawn-attack-around-king) (quote ((2 1.5) (14 1.5) (32 3))))
(my-engine (quote @weight-knight-attack-around-king) (quote ((2 1.5) (14 1.5) (32 3))))
(my-engine (quote @weight-bishop-attack-around-king) (quote ((2 1.5) (14 1.5) (32 3))))
(my-engine (quote @weight-rook-attack-around-king) (quote ((2 1.5) (14 1.5) (32 3))))
(my-engine (quote @weight-queen-attack-around-king) (quote ((2 1.5) (14 1.5) (32 3))))
(my-engine (quote @weight-king-attack-around-king) (quote ((2 5) (14 5) (32 0))))
(my-engine (quote @weight-pass-pawn) (quote ((2 30) (14 30) (32 20))))
(my-engine (quote @weight-protected-pass-pawn) (quote ((2 10) (14 10) (32 10))))
(my-engine (quote @weight-double-pawn) (quote ((2 -20) (14 -20) (32 -10))))
(my-engine (quote @weight-iso-pawn) (quote ((2 -5) (14 -5) (32 -10))))
(my-engine (quote @weight-pawn-shield) (quote ((2 0) (14 0) (32 1))))
(my-engine (quote @weight-bishop-pair) (quote ((2 60) (14 60) (32 60))))
(my-engine (quote @weight-bad-bishop) (quote ((2 0) (14 0) (32 -1.5))))
(my-engine (quote @weight-rook-pair) (quote ((2 20) (14 20) (32 10))))
(my-engine (quote @weight-rook-semiopen-fyle) (quote ((2 7.5) (14 7.5) (32 7.5))))
(my-engine (quote @weight-rook-open-fyle) (quote ((2 7.5) (14 7.5) (32 7.5))))
(my-engine (quote @weight-early-queen-starting) (quote ((2 0) (14 0) (32 -20))))
(my-engine (quote @weight-weak-square) (quote ((2 0) (14 0) (32 -5))))
(my-engine (quote @weight-castling) (quote ((2 0) (14 0) (32 20))))
(my-engine (quote @weight-abandoned-castling) (quote ((2 0) (14 0) (32 -110))))

;; Runs Engine.
(my-engine (quote @run))