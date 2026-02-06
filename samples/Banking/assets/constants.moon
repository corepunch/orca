header =
	iconSize: 28
	links: {
		-- {
		-- 	imgURL: "assets/icons/edit.svg"
		-- 	route: "/send-money"
		-- 	label: "Send Money"
		-- },
		{
			imgURL: "assets/icons/follow.svg"
			route: "/send-money"
			label: "Send Money"
		},
		{
			imgURL: "assets/icons/logout.svg"
			route: "/sign-out"
			label: "Send Money"
		},
	}

footer =
	iconSize: 40
	links: {
		{
			imgURL: "assets/icons/home.svg"
			route: "/"
			label: "Home"
		},
		{
			imgURL: "assets/icons/people.svg"
			route: "/send-money"
			label: "Send Money"
		},
		{
			imgURL: "assets/icons/edit.svg"
			route: "/new-tweet"
			label: "Write"
		}
		{
			imgURL: "assets/icons/chat.svg"
			route: "/tweets"
			label: "Tweets"
		},
		{
			imgURL: "assets/icons/bookmark.svg"
			route: "/search"
			label: "Bookmark"
		},
		-- {
		-- 	imgURL: "assets/icons/follow.svg"
		-- 	route: "/settings"
		-- 	label: "Posts"
		-- }
	}

return {
	:header
	:footer
}