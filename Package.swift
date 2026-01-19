// swift-tools-version: 6.2
import PackageDescription

let package = Package(
    name: "yomitandicts-cpp",
    platforms: [.iOS(.v18), .macOS(.v15)],
    products: [
        .library(name: "CYomitanDicts", targets: ["CYomitanDicts"]),
    ],
    dependencies: [
        .package(url: "https://github.com/facebook/zstd.git", from: "1.5.0"),
    ],
    targets: [
        .target(
            name: "Czip",
            path: "external/zip/src",
            sources: ["zip.c"],
            publicHeadersPath: "."
        ),
        .target(
            name: "CYomitanDicts",
            dependencies: [
                .product(name: "libzstd", package: "zstd"),
                "Czip"
            ],
            path: ".",
            sources: ["src"],
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("include"),
                .headerSearchPath("external/zip/src"),
            ],
            linkerSettings: [
                .linkedLibrary("sqlite3")
            ]
        ),
    ],
    cxxLanguageStandard: .cxx20
)
