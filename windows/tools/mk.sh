rm -f hresultwrap.pb.go hresultwrap
protoc --go_out=. hresultwrap.proto
go build hresultwrap.go hresultwrap.pb.go
