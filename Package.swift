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
            publicHeadersPath: ".",
            cSettings: [
                .define("ZIP_SHARED"),
                .define("ZIP_BUILD_SHARED"),
                .define("MINIZ_EXPORT", to: "__attribute__((visibility(\"hidden\")))"),
                .define("MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS"),
            ]
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
                .headerSearchPath("external/utfcpp/source"),
                .headerSearchPath("external/glaze/include"),
                .headerSearchPath("external/pthash/include"),
                .headerSearchPath("external/pthash/external/bits/include"),
                .headerSearchPath("external/pthash/external/bits/external/essentials/include"),
                .headerSearchPath("external/pthash/external/mm_file/include"),
                .headerSearchPath("external/pthash/external/xxHash"),
                .unsafeFlags(["-Wno-missing-braces"]),
            ],
            swiftSettings: [
                .interoperabilityMode(.Cxx)
            ]
        ),
    ],
    cxxLanguageStandard: .cxx2b
)
