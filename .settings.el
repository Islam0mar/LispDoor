
;;; package --- Summary
;;; Commentary: islam
;;; Project initial file:

(when (or (equal major-mode 'c++-mode) (equal major-mode 'c-mode))
  (cl-flet ((add-inc (folder)
                  (add-to-list 'ac-clang-flags
                               (concat "-I" (expand-file-name folder)))))
    (add-inc "include")
    (add-to-list 'ac-clang-flags "-Wno-unused-command-line-argument")
    (setf cmake-ide-project-dir (expand-file-name lds-dir))
    (setf cmake-ide-build-dir (expand-file-name (concat lds-dir "build")))
    t))

