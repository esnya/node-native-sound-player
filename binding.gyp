{
  "targets": [
    {
      "target_name": "wrapper",
      "sources": [ "wrapper/main.cc", "wrapper/common.cc", "wrapper/windows.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
