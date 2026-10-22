// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "SplitergollBatteries",
    platforms: [.macOS(.v13)],
    products: [.executable(name: "SplitergollBatteries", targets: ["SplitergollBatteries"])],
    targets: [
        .target(name: "BatteryCore"),
        .executableTarget(
            name: "SplitergollBatteries", dependencies: ["BatteryCore"],
            resources: [.copy("Resources")]),
        .testTarget(name: "BatteryCoreTests", dependencies: ["BatteryCore"]),
        .testTarget(name: "MenuBarTests", dependencies: ["SplitergollBatteries"]),
    ]
)
